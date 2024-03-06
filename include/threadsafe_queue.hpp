#pragma once

#include <queue>
#include <mutex>

template <typename T>
class ThreadsafeQueue {
public:
  void push(T value) {
    std::lock_guard<std::mutex> lock(_mut);
    _queue.push(value);
  }
  void pop() {
    std::lock_guard<std::mutex> lock(_mut);
    _queue.pop();
  }
  bool empty() {
    std::lock_guard<std::mutex> lock(_mut);
    return _queue.empty();
  }
  T front() {
    std::lock_guard<std::mutex> lock(_mut);
    return _queue.front();
  }
  int size() {
    std::lock_guard<std::mutex> lock(_mut);
    return _queue.size();
  }
private:
  std::mutex _mut;
  std::queue<T> _queue;
};
