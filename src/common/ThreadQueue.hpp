#pragma once

#include <queue>
#include <string>
#include <mutex>

template<class T>
class ThreadQueue {
 public:
  T pop();
  bool empty();
  void push(const T& s);
  void push(T&& s);
 private:
  std::mutex accessMutex;
  std::queue<T> queue;
};

template<class T>
T ThreadQueue<T>::pop() {
  accessMutex.lock();
  auto s = std::move(queue.front());
  queue.pop();
  accessMutex.unlock();
  return s;
}

template<class T>
bool ThreadQueue<T>::empty() {
  accessMutex.lock();
  auto r = queue.empty();
  accessMutex.unlock();
  return r;
}

template<class T>
void ThreadQueue<T>::push(const T& s) {
  accessMutex.lock();
  queue.push(s);
  accessMutex.unlock();
}

template<class T>
void ThreadQueue<T>::push(T&& s) {
  accessMutex.lock();
  queue.push(std::move(s));
  accessMutex.unlock();
}
