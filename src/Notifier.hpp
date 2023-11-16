#ifndef NOTIFIER_HPP
#define NOTIFIER_HPP

#include <queue>
#include <future>
#include <mutex>
#include <memory>

class Notifier{

private:

  std::queue<std::shared_ptr<std::promise<void>>> waitQueue;
  std::mutex lock;

public:

  std::future<void> add();
  std::future<void> remove();
  std::shared_ptr<std::promise<void>> pop();
  std::size_t size();

};


#endif