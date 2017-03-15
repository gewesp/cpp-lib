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
// References:
// [1] A very interesting paper by Herb Sutter on various options of
//     std::future design:
//     http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3630.pdf
// [2] http://stackoverflow.com/questions/23455104/why-is-the-destructor-of-a-future-returned-from-stdasync-blocking
//


#ifndef CPP_LIB_DISPATCH_H
#define CPP_LIB_DISPATCH_H

#include "cpp-lib/util.h"
#include "cpp-lib/sys/syslogger.h"

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

  // Noncopyable, but moveable
  thread_pool           (thread_pool const&) = delete;
  thread_pool& operator=(thread_pool const&) = delete;
  thread_pool           (thread_pool&&) = default;
  thread_pool& operator=(thread_pool&&) = default;

  int num_workers() const { return workers.size(); }

  // Second argument: Whether another task will follow this one.
  typedef std::pair<task, bool> task_and_continue;
  typedef cpl::util::safe_queue<task_and_continue> queue_type;

private:
  // We use a unique_ptr<> here to allow for move semantics of
  // the thread_pool object.  safe_queue isn't moveable because
  // it contains a std::mutex as a direct member.
  // Each worker thread needs a reference to the queue so we
  // need to guarantee that it stays at the same memory location
  // after its construction.
  std::unique_ptr<queue_type> tasks;
  std::vector<std::thread> workers;
};

// DEPRECATED:  Use thread_pool instead!
using dispatch_queue = thread_pool;

} // namespace dispatch

} // namespace cpl

//
// The only possible exceptions from packaged_task::operator() 
// according to the Standard are:
//
// std::future_error on the following error conditions:
// The stored task has already been invoked. The error category is 
// set to promise_already_satisfied.
//
// *this has no shared state. The error category is set to no_state.
//
// Both seem to be programming errors.
//

template<typename T> 
T cpl::dispatch::thread_pool::dispatch_returning(returning_task<T>&& t) {
  auto tfut = t.get_future();
  if (0 == num_workers()) {
    // Synchronous execution
    t();
  } else {
    // Asynchronous execution via wrapper task
    // cpl::dispatch::task wrapper([t_inner = std::move(t)] { t_inner(); });
    // t() should theoretically not throw---the exception should appear
    // in the get() below.
    cpl::dispatch::task wrapper([&t] { t(); });

    // Get the future before we give the wrapper away.
    auto wfut = wrapper.get_future();
    dispatch(std::move(wrapper));

    // Important: It seems we need to 'trigger' execution of the wrapper task
    // by tickling its future.
    try {
      wfut.get();
    } catch (std::exception const& e) {
      cpl::util::log::syslogger sl;
      sl << cpl::util::log::prio::ERR
         << "DISPATCH: Error in wrapper task: " << e.what()
         << std::endl;
    } catch (...) {
      cpl::util::log::syslogger sl;
      sl << cpl::util::log::prio::ERR
         << "DISPATCH: Unknown error in wrapper task"
         << std::endl;
    }
  }

  try {
    return tfut.get();
  } catch (std::exception const& e) {
    cpl::util::log::syslogger sl;
    sl << cpl::util::log::prio::ERR
       << "DISPATCH: Error in returning task: " << e.what()
       << std::endl;
  } catch (...) {
    cpl::util::log::syslogger sl;
    sl << cpl::util::log::prio::ERR
       << "DISPATCH: Unknown error in returning task"
       << std::endl;
  }
  return T();
}

#endif // CPP_LIB_DISPATCH_H
