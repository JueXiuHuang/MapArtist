#include "Notifier.hpp"

std::future<void> Notifier::add(){
  auto p = std::make_shared<std::promise<void>>();
  auto f = p->get_future();
  std::lock_guard<std::mutex> guard(lock);
  waitQueue.emplace(p);
  return f;
}

std::shared_ptr<std::promise<void>> Notifier::pop(){
  std::lock_guard<std::mutex> guard(lock);
  if(waitQueue.size() > 0){
    auto p = waitQueue.front();
    waitQueue.pop();
    return p;
  }else{
    return nullptr;
  }
}

std::size_t Notifier::size(){
  std::lock_guard<std::mutex> guard(lock);
  return waitQueue.size();
}