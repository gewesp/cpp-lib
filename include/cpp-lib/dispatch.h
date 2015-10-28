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
// References
// [1] man dispatch
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

struct dispatch_queue {
  // Creates a dispatch_queue with its own dispatch thread to
  // execute tasks.
  dispatch_queue();

  // Causes the dispatching thread to exit after all queued tasks have
  // been executed.
  ~dispatch_queue();

  // Adds a new task for execution, FIFO order.
  void dispatch_sync(task&& t);

  // Noncopyable
  dispatch_queue           (dispatch_queue const&) = delete;
  dispatch_queue& operator=(dispatch_queue const&) = delete;

private:
  // Second argument: Whether another task will follow this one.
  typedef std::pair<task, bool> task_and_continue;

  // Started from the constructor
  void thread_function();

  cpl::util::safe_queue<task_and_continue> tasks;
  std::thread th;
};

} // namespace dispatch

} // namespace cpl


#endif // CPP_LIB_DISPATCH_H
