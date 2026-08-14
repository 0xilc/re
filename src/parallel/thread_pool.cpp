#include "parallel/thread_pool.h"
#include "core/log.h"

namespace re {

ThreadPool::ThreadPool(int thread_count) {
  if (thread_count <= 0) {
    thread_count = static_cast<int>(std::thread::hardware_concurrency());
    if (thread_count <= 0) thread_count = 1;
  }

  workers_.reserve(static_cast<size_t>(thread_count));
  for (int i = 0; i < thread_count; ++i) {
    workers_.emplace_back(std::thread([this] { worker_loop(); }));
  }
  log_info("thread count: {}", workers_.size());
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
    ++generation_;
  }
  work_available_.notify_all();

  for (std::thread& t : workers_) {
    if (t.joinable()) t.join();
  }
}

void ThreadPool::worker_loop() {
  int last_generation = 0;
  for (;;) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      work_available_.wait(lock, [&] { return stop_ || generation_ != last_generation; });

      if (stop_) return;
      last_generation = generation_;
    }

    for (;;) {
      const int index = next_index_.fetch_add(1, std::memory_order_relaxed);
      if (index >= total_count_) break; 
      (*body_)(index);
    }

    if (active_.fetch_sub(1, std::memory_order_acq_rel ) == 1) {
      std::lock_guard<std::mutex> lock(mutex_);
      work_done_.notify_one();
    }
  }
}

void ThreadPool::parallel_for(int count, const std::function<void(int)>& body) {
  if (count <= 0) return;

  if (workers_.empty()) {
    for (int i = 0; i < count; ++i) body(i);
    return;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    body_ = &body;
    total_count_ = count;
    next_index_.store(0, std::memory_order_relaxed);
    active_.store(static_cast<int>(workers_.size()), std::memory_order_relaxed);
    ++generation_;
  }
  work_available_.notify_all();

  {
    std::unique_lock<std::mutex> lock(mutex_);
    work_done_.wait(lock, [&] { return active_.load(std::memory_order_acquire) == 0; });
  }

  body_ = nullptr;
  total_count_ = 0;
}

}