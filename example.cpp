#include<bits/stdc++.h>
#include"./include/loop/loop.h"

using namespace std;

namespace fortefibre {

uint64_t timestamp() {
    using namespace chrono;
    auto now = system_clock::now().time_since_epoch();
    return (uint64_t) duration_cast<milliseconds>(now).count();
}

} // namespace fortefibre

int main() {
    using namespace fortefibre;

    uint32_t count = 1;
    auto counter = Loop::set_interval([&] { cout << count++ << "sec" << endl; }, 1000);
    auto task = Loop::set_interval([&] { cout << "Hello world!" << endl; }, 2000);

    Loop::set_timeout([&] { task->stop(); }, 10000);
    Loop::set_timeout([&] { task->again(); }, 20000);
    Loop::set_timeout([&] {
        task->set_repeat(500);
        task->again();
    }, 25000);
    Loop::set_timeout([&] {
        task->stop();
        counter->stop();
        cout << "All timers are stopped!" << endl;
    }, 30000);

    Loop::run();
}
