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

#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>

#include <cstdlib>

#include "cpp-lib/dispatch.h"


namespace {

void test_dispatch() {
  cpl::dispatch::thread_pool disq;

  for (int i = 0; i < 50; ++i) {
    disq.dispatch([i]{ std::cout << i << std::endl; });
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
    auto const worker_func = 
        [m, &return_value, &themap_manager, &themap]() {
      int total = 0;
      for (int i = 0; i < m; ++i) {
        if (!return_value) {
                   themap_manager.dispatch
              ([&themap, i]() { ++themap[i];           });
        } else             {
          // std::cout << "push " << i << std::endl;
          total += themap_manager.dispatch_returning<int>
              ([&themap, i]() { ++themap[i]; return i; });
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

    workers.dispatch(worker_func);
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
    test_dispatch();

    // test_dispatch_manual(std::cout);

    test_dispatch_many(std::cout, true );
    test_dispatch_many(std::cout, false);
  } // global try
  catch( std::exception const& e )
  { std::cerr << e.what() << '\n' ; return 1 ; }

  return 0 ;
}
