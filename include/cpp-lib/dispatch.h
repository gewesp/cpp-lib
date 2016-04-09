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


#ifndef CPP_LIB_DISPATCH_H
#define CPP_LIB_DISPATCH_H

#include "cpp-lib/util.h"

#include <string>
#include <thread>
#include <utility>

namespace cpl {

namespace dispatch {

typedef std::function<void()> task;

struct thread_pool {
  // Creates and starts n threads to asynchronously execute tasks added 
  // by dispatch().
  // TODO: Allow to specify maximum number of waiting tasks before dispatch()
  // blocks?
  explicit thread_pool(int n = 1);

  // Causes the dispatching thread to exit after all queued tasks have
  // been executed.
  ~thread_pool();

  // Deprecated synonym for dispatch()!  The function is not synchronous,
  // it will return immediately.
  void dispatch_sync(task&& t) { dispatch(std::move(t)); }

  // Adds a new task for execution execution by the next available thread.
  // FIFO order is guaranteed if num_workers() == 1. Currently, dispatch()
  // never blocks, but see the for the constructor.
  void dispatch(task&& t);

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


#endif // CPP_LIB_DISPATCH_H
