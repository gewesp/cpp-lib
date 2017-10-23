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
#include <limits>
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
//

template <class T, bool BOUNDED = false> struct safe_queue {
  safe_queue(long capacity = std::numeric_limits<long>::max())
  : capacity_(capacity) {
    if (!BOUNDED && capacity != std::numeric_limits<long>::max()) {
      throw std::logic_error(
          "Attempt to construct unbounded queue with limited capacity");
    }
    if (capacity < 1) {
      throw std::runtime_error("Queue capacity must be >= 1");
    }
  }

  // Adds an element to the queue.  For unbounded queues, blocks only 
  // briefly in case a call to pop() or empty() is ongoing.
  // For bounded queues, blocks until space is available.
  void push(T&& t) {
    {
      std::unique_lock<std::mutex> lock{m};
      if (BOUNDED) {
        while (static_cast<long>(q.size()) == capacity()) {
          has_space.wait(lock);
        }
      }
      q.push(std::move(t));
    }
    // "(the lock does not need to be held for notification)"
    has_data.notify_one();
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
      has_data.wait(lock);
    }

    // lock is re-acquired after waiting, so we're good to go
    T t = std::move(q.front());
    q.pop();

    // TODO: Should unlock the mutex *here*, after pop()
    if (BOUNDED) {
      has_space.notify_one();
    }
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

  // Returns true iff the queue is bounded and full.
  bool full() const {
    if (!BOUNDED) {
      return false;
    }
    std::unique_lock<std::mutex> lock{m};
    return q.size() == capacity();
  }

  long capacity() const {
    if (!BOUNDED) {
      throw std::logic_error(
          "Attempt to obtain capacity of unbounded queue");
    }
    return capacity_;
  }

private:
  std::queue<T> q;
  long capacity_;
  mutable std::mutex m;
  std::condition_variable has_data;
  std::condition_variable has_space;
};


} // namespace util

} // namespace cpl

#endif // CPP_LIB_SAFE_QUEUE_H
