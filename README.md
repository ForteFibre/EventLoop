# EventLoop

イベントループのライトウェイトでシングルヘッダオンリーなライブラリ

```cpp
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

    Loop::set_timeout([&] {
        task->stop();
        counter->stop();
        cout << "All timers are stopped!" << endl;
    }, 10000);

    Loop::run();
}
```

## インストール方法

インクルードディレクトリに `include/loop/loop.h` を置くだけで使えます．

## 基本的な使い方

EventLoopを使うには `fortefibre` 名前空間の下に現在時刻をミリ秒で返す `timestamp` 関数を定義しなければなりません．`<chrono>` ヘッダが利用可能な環境では以下のように定義されることでしょう．

```cpp
namespace fortefibre {

uint64_t timestamp() {
    using namespace chrono;
    auto now = system_clock::now().time_since_epoch();
    return (uint64_t) duration_cast<milliseconds>(now).count();
}

} // namespace fortefibre
```

あるいは，例えば[stm32plus](https://github.com/andysworkshop/stm32plus)をお使いなら次のような定義になることでしょう．

```cpp
namespace fortefibre {

uint64_t timestamp() {
    return stm32plus::MillisecondTimer::millis();
}

}
```

### `Loop::run`

```cpp
void Loop::run();
```

イベントループを実行します．現状，この関数から処理が返ってくることはありません．

### `Loop::set_interval`

```cpp
template<typename Func>
Loop::Timer::Ptr Loop::set_interval(Func &&func, uint64_t timeout);
```

`timeout` ミリ秒ごとに `func` を実行されるように `Timer` をイベントループの実行対象に加えます．

### `Loop::set_timeout`

```cpp
template<typename Func>
Loop::Timer::Ptr Loop::set_timeout(Func &&func, uint64_t timeout)
```

`timeout` ミリ秒後に一回だけ `func` を実行されるように `Timer` をイベントループの実行対象に加えます．

### `Loop::Timer`

```cpp
class Loop::Timer;
```

時間に関係するタスクです．`Loop::Timer::Ptr` は `Loop::Timer` のスマートポインタです．

### `Loop::Timer::create`

```cpp
template<typename Func>
static Loop::Timer::Ptr Loop::Timer::create(Func &&func);
```

`Timer` を作成するファクトリ関数です．作成された時点では `Timer` はイベントループの実行対象に含まれていません．作成された `Timer` をイベントループの実行対象に含まれるようにするには `start` を呼ばなければなりません．

### `Loop::Timer::start`

```cpp
void Loop::Timer::start(uint64_t timeout, uint64_t repeat = 0);
```

`Timer` を動作させます．より具体的には，`Timer` をイベントループの実行対象に加えます．

- `repeat` が0のとき，`timeout` ミリ秒後に一回だけ関数を実行するようイベントループにスケジュールされます．
- `repeat` が1以上のとき，`timeout` ミリ秒後まで待機したのち，`repeat` ミリ秒ごとに関数を実行するようイベントループにスケジュールされます．

### `Loop::Timer::stop`

```cpp
void Loop::Timer::stop();
```

`Timer` を停止させます．より具体的には，`Timer` をイベントループの実行対象から外します．
