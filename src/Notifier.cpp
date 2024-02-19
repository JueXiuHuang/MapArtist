#include "Notifier.hpp"

void Notifier::wait()
{
  std::unique_lock<std::mutex> lock(mtx);
  lock.lock();
  std::size_t cur_id = id;
  lock.unlock();
  cv.wait(lock, [&]()
          { return id > cur_id; });
}

void Notifier::notify()
{
  std::lock_guard<std::mutex> lock(mtx);
  id++;
  cv.notify_one();
}

void Notifier::notify_all()
{
  std::lock_guard<std::mutex> lock(mtx);
  id++;
  cv.notify_all();
}

std::size_t Notifier::getID()
{
  return id;
}

Notifier::Notifier() : id(0) {}