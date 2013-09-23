#pragma once
#ifndef THREADPOOL_HPP
# define THREADPOOL_HPP

#include <cstddef>

#include <atomic>

#include <condition_variable>

#include <mutex>

#include <thread>

#include <utility>

#include <vector>

#include "generic/delegate.hpp"

class thread_pool
{
public:
  using delegate_type = ::delegate<void ()>;

  thread_pool() = delete;

  thread_pool(thread_pool const&) = delete;

  thread_pool& operator=(thread_pool const&) = delete;

  static void execute(delegate_type);

  static void init(unsigned);

  static void exit();

private:
  static void run();

  static void spawn_thread();

private:
  static ::std::mutex cm_;
  static ::std::condition_variable cv_;

  static ::std::atomic_int fc_;
  static ::std::atomic_bool qf_;

  static ::std::vector<delegate_type> delegates_;
};

//////////////////////////////////////////////////////////////////////////////
inline void thread_pool::execute(delegate_type e)
{
  if (fc_.fetch_sub(1) <= 0)
  {
    spawn_thread();
  }
  // else do nothing

  {
    ::std::lock_guard<decltype(cm_)> l(cm_);

    delegates_.emplace_back(::std::move(e));
  }

  cv_.notify_one();
}

//////////////////////////////////////////////////////////////////////////////
inline void thread_pool::exit()
{
  ::thread_pool::qf_.store(true);

  cv_.notify_all();
}

//////////////////////////////////////////////////////////////////////////////
inline void thread_pool::spawn_thread()
{
  ::std::thread(run).detach();
}

#endif // THREADPOOL_HPP
