//
// Copyright 2017 and onwards, KISS Technologies GmbH, Switzerland
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
// Component: UTIL
//

#ifndef CPP_LIB_SAFE_QUEUE_H
#define CPP_LIB_SAFE_QUEUE_H


#include <condition_variable>
#include <mutex>
#include <queue>


namespace cpl {

namespace util {
 
////////////////////////////////////////////////////////////////////////
// Some simple thread-safe structures
////////////////////////////////////////////////////////////////////////

//
// A thread-safe queue that can have multiple writers and multiple
// readers.
//
// Heavily modified, based on a reply from
// http://stackoverflow.com/questions/15278343/c11-thread-safe-queue
// See also:
// http://en.cppreference.com/w/cpp/thread/condition_variable
//
// TODO:
// * Add a safeguard against destruction when there's still a reader or 
//   writer?  Just acquiring a lock on the mutex in the destructor doesn't
//   work because wait() unlocks the mutex.
// * Add maximum size and a full() condition.
//

template <class T> struct safe_queue {

  // Adds an element to the queue.  Blocks only briefly in case a
  // call to pop() or empty() is ongoing.
  void push(T&& t) {
    {
      std::lock_guard<std::mutex> lock{m};
      q.push(std::move(t));
    }
    // "(the lock does not need to be held for notification)"
    c.notify_one();
  }

  // Waits for an element to become available, removes it from
  // the queue and returns it.  If a previous call to empty()
  // returned false and there is only one consumer, pop()
  // does not block.
  T pop() {
    std::unique_lock<std::mutex> lock{m};

    // If q.empty(), this was a spurious wakeup
    // http://en.cppreference.com/w/cpp/thread/condition_variable/wait
    while (q.empty()) {
      c.wait(lock);
    }

    // lock is re-acquired after waiting, so we're good to go
    T t = std::move(q.front());
    q.pop();
    return t;
  }

  // Deprecated synonym for pop().  Use pop() instead.
  T pop_front() {
    return pop();
  }

  // Returns true iff the queue is empty.
  bool empty() const {
    std::unique_lock<std::mutex> lock{m};
    return q.empty();
  }

private:
  std::queue<T> q;
  std::mutex m;
  std::condition_variable c;
};


} // namespace util

} // namespace cpl

#endif // CPP_LIB_SAFE_QUEUE_H
