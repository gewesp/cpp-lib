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


// Make sure that tasks is initialized before the thread is
// started!
cpl::dispatch::dispatch_queue::dispatch_queue() 
  : tasks{},
    th{&dispatch_queue::thread_function, this}
{}

cpl::dispatch::dispatch_queue::~dispatch_queue() {
  // Signal 'EOF'
  tasks.push(task_and_continue{[]{}, false});
  th.join();
}

void cpl::dispatch::dispatch_queue::dispatch_sync(cpl::dispatch::task&& t) {
  tasks.push(task_and_continue{std::move(t), true});
}

void cpl::dispatch::dispatch_queue::thread_function() {
  do {
    auto const tac = tasks.pop_front();
    if (!tac.second) {
      return;
    } else {
      tac.first();
    }
  } while (true);
}
