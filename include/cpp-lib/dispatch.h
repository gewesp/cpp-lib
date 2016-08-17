//
// Copyright 2015 KISS Technologies GmbH, Switzerland
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Component: DISPATCH
//
// C++11 implementation of the part of Grand Central Dispatch that's
// well-defined and useful.
//
// Usage:
// * For synchronising access to a contended resource, use a queue with
//   one worker thread:
// 
//   resource res;
//   thread_pool serializer(1);
//   void process() {
//     while (auto x = get_input()) { 
//       serializer.push([x, &res] { res.process_input(x); });
//     }
//   }
//   
//   std::thread t1( process );
//   std::thread t2( process );
//   // Calls to res.process_input() are serialised in q's worker thread
//
// * For distributing independent tasks to worker n threads, e.g. download
//   files in parallel:
//
//   void download_files(std::vector<std::string> const& files,
//                       const int n_threads) {
//     thread_pool pool(n_threads);
//     for (auto const& f : files) {
//       pool.dispatch([f] { download(f); });
//     }
//     // thread_pool destructor waits for all downloads to finish
//   }
//
// Notes:
// * Carefully consider call by reference/value in capture lists!
//
// TODO:
// * The interface is still not very nice.  Use templated dispatch() with
//   raw function objects?
// * Maximum queue size with intelligent management
// * The use of std::future for wrapper task and/or tasks without
//   return value may be overkill?
//


#ifndef CPP_LIB_DISPATCH_H
#define CPP_LIB_DISPATCH_H

#include "cpp-lib/util.h"

#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <future>

namespace cpl {

namespace dispatch {

// Note: packaged_task constructors are explicit!
typedef std::packaged_task<void()> task;

// Task with return value of type T
template<typename T> using returning_task = std::packaged_task<T()>;

struct thread_pool {
  // Creates and starts n threads to asynchronously execute tasks added 
  // by dispatch().
  // If n == 0, no threads are created and dispatch() calls will execute
  // the tasks in the calling thread.
  // TODO: Allow to specify maximum number of waiting tasks before dispatch()
  // blocks?
  explicit thread_pool(int n = 1);

  // Causes the dispatching thread to exit after all queued tasks have
  // been executed.
  ~thread_pool();

  // Deprecated synonym for dispatch()!  The function is not synchronous,
  // it will return immediately.
  void dispatch_sync(task&& t) { dispatch(std::move(t)); }

  // If num_workers() >= 1, adds a new task for execution execution by the 
  // next available thread.  If num_workers() == 0, executes t in the
  // calling thread. FIFO order is guaranteed if num_workers() <= 1
  // Note: cpl::dispatch::task constructors are explicit!
  void dispatch(task&& t);

  // As for dispatch(), adds t for execution to the FIFO or executes 
  // it in the calling thread.
  //
  // Blocks the calling thread until the function returns and forwards
  // the return value.  Returns a default constructed value if t 
  // or T's copy constructor throws.
  //
  // TODO: Better use of move semantics---Use C++14 generalized capture.
  // TODO: Log exceptions ot syslog.
  template<typename T> T dispatch_returning(returning_task<T>&& t);

  // Noncopyable
  thread_pool           (thread_pool const&) = delete;
  thread_pool& operator=(thread_pool const&) = delete;

  int num_workers() const { return workers.size(); }

private:
  // Second argument: Whether another task will follow this one.
  typedef std::pair<task, bool> task_and_continue;

  // Started from the constructor
  void thread_function();

  cpl::util::safe_queue<task_and_continue> tasks;
  std::vector<std::thread> workers;
};

// DEPRECATED:  Use thread_pool instead!
using dispatch_queue = thread_pool;

} // namespace dispatch

} // namespace cpl


// http://en.cppreference.com/w/cpp/thread/condition_variable
template<typename T> 
T cpl::dispatch::thread_pool::dispatch_returning(returning_task<T>&& t) {
  auto result = t.get_future();

  if (0 == num_workers()) {
    t();
    return result.get();
  }

  cpl::dispatch::task wrapper([&t] () {
    try { 
      t();
    } catch (std::exception const& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      // TODO: Log
    } catch (...) {
      std::cerr << "ERROR: nonstandard exception" << std::endl;
      // TODO: Log
    }
  });

  auto fut = wrapper.get_future();
  this->dispatch(std::move(wrapper));
  // Important: It seems we need to 'trigger' execution of the wrapper task
  // by tickling its future.
  // The whole future business looks a bit unfinished, cf.
  // http://stackoverflow.com/questions/23455104/why-is-the-destructor-of-a-future-returned-from-stdasync-blocking
  fut.get();

  // Wait for result and return it.
  return result.get();
}

#endif // CPP_LIB_DISPATCH_H
