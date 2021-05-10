#pragma once

#include<cstdint>
#include<functional>
#include<memory>
#include<limits>
#include<set>

namespace fortefibre {

uint64_t timestamp();

template<typename T>
class NonCopyable {
    NonCopyable() { }

    friend T;

public:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
};

class Runnable {
    template<typename Func>
    class Impl;

public:
    virtual void run() = 0;
    virtual ~Runnable() { }

    using Ptr = std::shared_ptr<Runnable>;

    template<typename F>
    static Ptr create(F &&func)
    { return std::make_shared<Impl<F>>(std::forward<F>(func)); }
};

template<typename Func>
class Runnable::Impl : public Runnable {
    Func _func;

public:
    Impl(Func func): _func(func) { }

    void run() override
    { _func(); }
};

class Loop : NonCopyable<Loop> {
public:
    class TimerTask : public std::enable_shared_from_this<TimerTask>, NonCopyable<TimerTask> {
        Runnable::Ptr _runnable;
        Loop *_loop;
        uint64_t _id;
        uint64_t _timeout = 0;
        uint64_t _repeat = 0;

        template<typename Func>
        TimerTask(Func &&func, Loop *loop)
            : _runnable(Runnable::create(std::forward<Func>(func)))
            , _loop(loop) { }

    public:
        using Ptr = std::shared_ptr<TimerTask>;

        struct PtrOrd {
            bool operator()(const TimerTask::Ptr a, const TimerTask::Ptr b) const
            { return a->_timeout != b->_timeout ? a->_timeout < b->_timeout : a->_id < b->_id; }
        };

        template<typename Func>
        static Ptr create(Func &&func, Loop *loop = singleton())
        {
            struct TimerTaskHelper : TimerTask {
                TimerTaskHelper(Func &&func, Loop *loop): TimerTask(std::forward<Func>(func), loop) { }
            };
            return std::make_shared<TimerTaskHelper>(std::forward<Func>(func), loop);
        }

        void start(uint64_t timeout, uint64_t repeat = 0) noexcept
        {
            stop();
            _timeout = timestamp() + timeout;
            if (_timeout < timeout) _timeout = std::numeric_limits<decltype(_timeout)>::max();
            _repeat = repeat;
            _id = _loop->_timer_count++;
            _loop->_timer_tasks.insert(shared_from_this());
        }

        void stop() noexcept
        { _loop->_timer_tasks.erase(shared_from_this()); }

        void again() noexcept
        {
            if (!_repeat) return;
            stop();
            start(_repeat, _repeat);
        }

        void set_repeat(uint64_t repeat) noexcept
        { _repeat = repeat; }

        bool expired() const noexcept
        { return _timeout < _loop->_timestamp; }

        void run() noexcept
        { _runnable->run(); }
    };

private:
    std::set<TimerTask::Ptr, TimerTask::PtrOrd> _timer_tasks;
    uint64_t _timer_count = 0;
    uint64_t _timestamp = 0;

public:
    void update_time()
    { _timestamp = timestamp(); }

    void run_timers() noexcept
    {
        if (_timer_tasks.empty()) return;

        auto task = *_timer_tasks.begin();
        if (!task->expired()) return;

        task->stop();
        task->again();
        task->run();
    }

    template<typename Func>
    static auto set_timeout(Func &&func, uint64_t timeout, Loop *loop = singleton()) noexcept
    {
        auto res = TimerTask::create(std::forward<Func>(func), loop);
        res->start(timeout);
        return res;
    }

    template<typename Func>
    static auto set_interval(Func &&func, uint64_t timeout, Loop *loop = singleton()) noexcept
    {
        auto res = TimerTask::create(std::forward<Func>(func), loop);
        res->start(timeout, timeout);
        return res;
    }

    static void run(Loop *loop = singleton()) noexcept
    {
        while (true) {
            loop->update_time();
            loop->run_timers();
        }
    }

    static Loop *singleton()
    {
        static Loop loop;
        return &loop;
    }
};

}
