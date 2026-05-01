#pragma once
#include <coroutine>
#include <exception>

template <typename T>
struct Generator {
  struct promise_type {
    T value{};
    Generator get_return_object() { return Generator{std::coroutine_handle<promise_type>::from_promise(*this)}; }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    std::suspend_always yield_value(T v) noexcept {
      value = v;
      return {};
    }
    void return_void() noexcept {}
    void unhandled_exception() noexcept { std::terminate(); }
  };

  std::coroutine_handle<promise_type> handle;

  explicit Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
  ~Generator() {
    if (handle) handle.destroy();
  }

  Generator(const Generator&) = delete;
  Generator& operator=(const Generator&) = delete;
  Generator(Generator&& o) noexcept : handle(o.handle) { o.handle = nullptr; }

  // advance one step, returns false when the coroutine is exhausted
  bool next() {
    if (!handle || handle.done()) return false;
    handle.resume();
    return !handle.done();
  }

  T current() const { return handle.promise().value; }

  // implement iterator

  // struct Iterator {
  //   std::coroutine_handle<promise_type> handle;
  //   void operator++() { handle.resume(); }
  //   const T& operator*() const { return handle.promise().current_value; }
  //   bool operator==(std::default_sentinel_t) const { return handle.done(); }
  // };
  //
  // Iterator begin() {
  //   handle.resume();
  //   return {handle};
  // }
  //
  // std::default_sentinel_t end() { return {}; }
};
