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

#include "cpp-lib/dispatch.h"

void thread_function(cpl::dispatch::thread_pool::queue_type& tasks);

// Make sure that tasks is initialized before the thread is
// started!
// Initialize thread pool
cpl::dispatch::thread_pool::thread_pool(int const n_threads)
  : tasks(std::make_unique<queue_type>()) {
  assert(tasks.get());  
  cpl::util::verify(n_threads >= 0, 
      "thread pool: number of worker threads must be >= 0");
  workers.reserve(n_threads);
  for (int i = 0; i < n_threads; ++i) {
    // Each thread gets a reference to the task queue.  The queue
    // is held indirectly via a pointer to enable move semantics
    // of the thread_pool.
    workers.push_back(std::thread(thread_function, std::ref(*tasks.get())));
  }
}

// TODO: allow detaching?
cpl::dispatch::thread_pool::~thread_pool() {
  // Have we been moved from?  std::unique_ptr<> is
  // guaranteed to be empty after a move operation.
  if (!tasks.get()) {
    return;
  }
  // Signal 'EOF' to all workers---one thread will pop
  // only one task wiht continue set to false
  for (int i = 0; i < num_workers(); ++i) {
    task empty([]{});
    tasks->push(task_and_continue{std::move(empty), false});
  }
  // Join all workers
  for (int i = 0; i < num_workers(); ++i) {
    workers[i].join();
  }
}

void cpl::dispatch::thread_pool::dispatch(cpl::dispatch::task&& t) {
  assert(tasks.get());  
  if (num_workers() > 0) {
    tasks->push(task_and_continue{std::move(t), true});
  } else {
    // Direct execution, re-throw exceptions (on get())
    t();
    t.get_future().get();
  }
}

void thread_function(cpl::dispatch::thread_pool::queue_type& tasks) {
  do {
    auto tac = std::move(tasks.pop_front());
    if (!tac.second) {
      return;
    } else {
      // Execute task.  This should not throw, exceptions are stored in
      // the 'shared state'.
      tac.first();
    }
  } while (true);
}
