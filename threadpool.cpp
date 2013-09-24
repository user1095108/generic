#include <cstdlib>

#include <utility>

#include "threadpool.hpp"

::std::mutex thread_pool::cm_;
::std::condition_variable thread_pool::cv_;

::std::atomic_int thread_pool::fc_;

::std::atomic_bool thread_pool::qf_;

::std::vector<::thread_pool::delegate_type> thread_pool::delegates_;

//////////////////////////////////////////////////////////////////////////////
void thread_pool::init(unsigned size)
{
  size = ::std::max(decltype(size)(1), size);

  delegates_.reserve(size);

  fc_.store(size, ::std::memory_order_relaxed);

  while (size--)
  {
    spawn_thread();
  }
}

//////////////////////////////////////////////////////////////////////////////
void thread_pool::run()
{
  for (;;)
  {
    delegate_type c;

    {
      ::std::unique_lock<decltype(cm_)> l(cm_);

      while (!qf_.load(::std::memory_order_relaxed) && delegates_.empty())
      {
        cv_.wait(l);
      }

      if (qf_.load(::std::memory_order_relaxed))
      {
        break;
      }
      else
      {
        c = ::std::move(delegates_.back());

        delegates_.pop_back();
      }
    }

    c();

    fc_.fetch_add(1, ::std::memory_order_relaxed);
  }
}
