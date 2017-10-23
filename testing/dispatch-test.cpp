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

#include <array>
#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>

#include <cstdlib>

#include "cpp-lib/dispatch.h"

#include "cpp-lib/assert.h"


namespace {

template<bool BOUNDED>
void producer(
    std::reference_wrapper<cpl::util::safe_queue<int, BOUNDED>> out_ref,
    int const N_NUMBERS) {
  auto& out = out_ref.get();
  for (int i = 0; i < N_NUMBERS; ++i) {
    int ii = i;
    out.push(std::move(ii));
  }
}

template<bool BOUNDED>
void worker(
    std::reference_wrapper<cpl::util::safe_queue<int, BOUNDED>>  in_ref,
    std::reference_wrapper<cpl::util::safe_queue<int, BOUNDED>> out_ref) {
  auto& in  =  in_ref.get();
  auto& out = out_ref.get();
  int i;
  while ((i = in.pop()) >= 0) {
    out.push(std::move(i));
  }
}

template<bool BOUNDED>
void consumer(
    cpl::util::safe_queue<int, BOUNDED>& in,
    std::ostream& os,
    const long N_PRODUCERS,
    const long N_NUMBERS) {

  std::vector<int> numbers(N_NUMBERS);

  long count = 0;
  while (count < N_PRODUCERS * N_NUMBERS) {
    const int i = in.pop();
    // std::cout << "C";
    ++numbers.at(i);
    ++count;
  }

  for (int i = 0; i < N_NUMBERS; ++i) {
    if (N_PRODUCERS != numbers.at(i)) {
      os << "ERROR: "
         << "Number " << i
         << ": " 
         << numbers.at(i)
         << " occurrances" << std::endl;
      throw std::runtime_error("safe_queue test failed");
    }
  }
}

// Test multi-producer multi-consumer queue
template<bool BOUNDED>
void test_safe_queue(
  std::ostream& os,  
  int const N_PRODUCERS, 
  int const N_WORKERS  ,
  int const N_NUMBERS  ,
  long const QUEUE_SIZE = std::numeric_limits<long>::max()) {

  os << "Testing "
     << (BOUNDED ? "bounded" : "unbounded")
     << " safe_queue with "
     << N_PRODUCERS << " producer(s), "
     << N_WORKERS   << " worker(s), "
     << N_NUMBERS   << " number(s)";
  if (BOUNDED) {
    os << ", size " << QUEUE_SIZE;
  }
  os << " ... " << std::flush;

  std::vector<std::thread> producer_threads;
  std::vector<std::thread>   worker_threads;

  cpl::util::safe_queue<int, BOUNDED> producer_worker  (QUEUE_SIZE);
  cpl::util::safe_queue<int, BOUNDED>   worker_consumer(QUEUE_SIZE);

  for (int i = 0; i < N_PRODUCERS; ++i) {
    producer_threads.push_back(
        std::thread(producer<BOUNDED>, std::ref(producer_worker), N_NUMBERS));
  }

  for (int i = 0; i < N_WORKERS; ++i) {
    worker_threads.push_back(
        std::thread(worker<BOUNDED>,
        std::ref(producer_worker), 
        std::ref(worker_consumer)));
  }

  consumer<BOUNDED>(worker_consumer, os, N_PRODUCERS, N_NUMBERS);

  // Producers should be joinable once the consumer
  // has seen enough (N_PRODUCERS * N_NUMBERS) inputs.
  // os << "Joining producers" << std::endl;
  for (auto& t : producer_threads) {
    t.join();
  }

  // Now, we can tell the workers to go home.  Each one will
  // pop a -1 and exit.  Better to do that here than
  // in the producers because the producers don't know
  // the number of workers.
  // os << "Laying off workers" << std::endl;
  for (int i = 0; i < N_WORKERS; ++i) {
    producer_worker.push(-1);
  }

  // After this, all workers should be joinable.
  // os << "Joining workers" << std::endl;
  for (auto& t : worker_threads) {
    t.join();
  }

  os << "OK" << std::endl;
}

void test_safe_queue(std::ostream& os) {
  // Unbounded case first
  // N_PRODUCERS, N_WORKERS, N_NUMBERS  
  test_safe_queue<false>(os, 5, 10, 100000);
  test_safe_queue<false>(os, 10, 5, 100000);
  test_safe_queue<false>(os, 5, 1 , 100000);
  test_safe_queue<false>(os, 1, 5 , 100000);
  test_safe_queue<false>(os, 2, 5 , 100000);

  test_safe_queue<false>(os, 2, 5, 1);
  // Bounded case
  // N_PRODUCERS, N_WORKERS, N_NUMBERS, QUEUE_SIZE  
  test_safe_queue<true>(os, 1, 10, 1000, 10);
  test_safe_queue<true>(os, 2, 9, 1000, 10);
  test_safe_queue<true>(os, 3, 8, 1000, 10);
  test_safe_queue<true>(os, 4, 7, 1000, 10);
  test_safe_queue<true>(os, 6, 3, 1000, 10);
  test_safe_queue<true>(os, 8, 2, 1000, 10);
  test_safe_queue<true>(os, 100, 2, 1000, 10);
  test_safe_queue<true>(os, 100, 1, 1000, 10);
  test_safe_queue<true>(os, 1, 100, 1000, 10);

  // Some corner cases
  test_safe_queue<true>(os, 1, 100, 1000, 1000000);
  test_safe_queue<true>(os, 1, 100, 1000, 1);
  test_safe_queue<true>(os, 1, 100, 0, 1);
  test_safe_queue<true>(os, 111, 100, 1, 1);
  test_safe_queue<true>(os, 111, 100, 1, 2);
  test_safe_queue<true>(os, 111, 100, 1, 7);
}

void test_dispatch() {
  // Also tests move semantics
  cpl::dispatch::thread_pool tp1;

  for (int i = 0; i < 25; ++i) {
    cpl::dispatch::task t([i]{ std::cout << i << std::endl; });
    tp1.dispatch(std::move(t));
  }

  auto tp2 = std::move(tp1);

  auto fun = [&tp1] { tp1.dispatch(cpl::dispatch::task([] {})); } ;
  cpl::util::verify_throws("moved-from", fun);

  for (int i = 25; i < 50; ++i) {
    cpl::dispatch::task t([i]{ std::cout << i << std::endl; });
    tp2.dispatch(std::move(t));
  }
}


// E.g., w workers execute n tasks incrementing m elements each in a 
// map<int,int> by dispatching to a single 'map manager'.
void test_dispatch_n(std::ostream& os, int const w, int const n, int const m,
    bool const return_value) {
  std::map<int, int> themap;

  os << "Map increment test: "
     << w << " worker(s), "
     << n << " task(s), "
     << m << " element(s), "
     << "return value: " << return_value
     << std::endl;
    

  {

  // One single worker handling themap
  cpl::dispatch::thread_pool themap_manager;

  // w workers, all dispatching tasks to themap_manager
  cpl::dispatch::thread_pool workers(w);

  for (int j = 0; j < n; ++j) {
    auto worker_func = 
        [m, &return_value, &themap_manager, &themap]() {
      int total = 0;
      for (int i = 0; i < m; ++i) {
        if (!return_value) {
          cpl::dispatch::task t([&themap, i]() { ++themap[i]; });
                   themap_manager.dispatch(std::move(t));
        } else             {
          // std::cout << "push " << i << std::endl;
          total += themap_manager.dispatch_returning<int>(
              cpl::dispatch::returning_task<int>([&themap, i]() { ++themap[i]; return i; }));
        }

      }
      if (return_value) {
        int const expected = m * (m - 1) / 2;
        if (total != expected) {
          // TODO: Make thread_pool handle exceptions and use them
          // here
          std::cerr << "error in thread_pool test: wrong sum"
                    << ": expected "
                    << expected
                    << "; actual: "
                    << total
                    << std::endl;
          std::exit(1);
        }
      }
    };

    workers.dispatch(cpl::dispatch::task(worker_func));
  }

  }
  // The thread_pool destructor causes worker threads to be joined.
  // Therefore, themap is now free

  for (int i = 0; i < m; ++i) {
    // n tasks incremented each element by 1
    cpl::util::verify(n == themap[i], 
        "error in thread_pool test: wrong map element");
  }
  os << "test ok" << std::endl;
}

#if 0
void test_dispatch_manual(std::ostream& os) {
  os << "Enter number of workers, tasks, elements: ";
  int w = 0, t = 0, e = 0;
  while (std::cin >> w >> t >> e) {
    test_dispatch_n(os, w, t, e, true);
  }
}
#endif


void test_dispatch_many(
    std::ostream& os, bool const return_value) {
  // Currently fails in most cases on MacOS/clang with 1, 40, 40,
  // opt mode
  test_dispatch_n(os, 1  , 40, 40, return_value);
  test_dispatch_n(os, 1  , 1, 3, return_value);
  test_dispatch_n(os, 1  , 3, 3, return_value);

  test_dispatch_n(os, 1  , 100, 10000, return_value);
  test_dispatch_n(os, 3  , 100, 10000, return_value);
  test_dispatch_n(os, 10 , 100, 10000, return_value);
  test_dispatch_n(os, 100, 100, 10000, return_value);
}

} // anonymous namespace


// int main( int const argc , char const * const * const argv ) {
int main() {

  try {
    test_safe_queue(std::cout);
    test_dispatch();

    // test_dispatch_manual(std::cout);

    test_dispatch_many(std::cout, true );
    test_dispatch_many(std::cout, false);
  } // global try
  catch( std::exception const& e )
  { std::cerr << e.what() << '\n' ; return 1 ; }

  return 0 ;
}
