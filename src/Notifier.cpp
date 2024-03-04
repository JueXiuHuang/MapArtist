// Copyright 2024 JueXiuHuang, ldslds449

#include "./Notifier.hpp"  // NOLINT

void Notifier::wait() {
  std::unique_lock<std::mutex> lock(mtx);
  lock.lock();
  std::size_t cur_id = id;
  lock.unlock();
  cv.wait(lock, [&]() { return id > cur_id; });
}

void Notifier::notify() {
  std::lock_guard<std::mutex> lock(mtx);
  id++;
  cv.notify_one();
}

void Notifier::notify_all() {
  std::lock_guard<std::mutex> lock(mtx);
  id++;
  cv.notify_all();
}

Notifier::Notifier() : id(0) {}
