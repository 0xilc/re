#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace re {

class ThreadPool {
public:
  explicit ThreadPool(int thread_count = 0);
  ~ThreadPool();

  ThreadPool(const ThreadPool&)             = delete;
  ThreadPool& operator=(const ThreadPool&)  = delete;

  int thead_count() const { return static_cast<int>( workers_.size()); }
  void parallel_for(int count, const std::function<void(int)>& body);

private:
  void worker_loop();
  
  std::vector<std::thread> workers_;
  std::mutex               mutex_;
  std::condition_variable  work_available_;
  std::condition_variable  work_done_;

  const std::function<void(int)>* body_ = nullptr;
  std::atomic<int>                next_index_ = 0;
  int                             total_count_ = 0;
  std::atomic<int>                active_ = 0;
  int                             generation_ = 0;
  bool                            stop_ = false;
};

}