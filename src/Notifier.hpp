// Copyright 2024 JueXiuHuang, ldslds449

#ifndef SRC_NOTIFIER_HPP_
#define SRC_NOTIFIER_HPP_

#include <chrono>
#include <condition_variable>
#include <mutex>

class Notifier {
 private:
  std::mutex mtx;
  std::condition_variable cv;
  std::size_t id;

 public:
  Notifier();
  void wait();

  template <typename Rep, typename Period>
  bool wait_for(const std::chrono::duration<Rep, Period> &duration) {
    std::unique_lock<std::mutex> lock(mtx);
    lock.lock();
    std::size_t cur_id = id;
    lock.unlock();
    return cv.wait_for(lock, duration, [&]() { return id > cur_id; });
  }

  template <typename Clock>
  bool wait_until(const std::chrono::time_point<Clock> &time_point) {
    std::unique_lock<std::mutex> lock(mtx);
    lock.lock();
    std::size_t cur_id = id;
    lock.unlock();
    return cv.wait_until(lock, time_point, [&]() { return id > cur_id; });
  }

  void notify();
  void notify_all();
};

#endif  // SRC_NOTIFIER_HPP_
