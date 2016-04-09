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
void test_dispatch_n(std::ostream& os, int const w, int const n, int const m) {
  std::map<int, int> themap;

  os << "Map incremente test: "
     << w << " worker(s), "
     << n << " task(s), "
     << m << " element(s)"
     << std::endl;
    

  {

  cpl::dispatch::thread_pool themap_manager;
  cpl::dispatch::thread_pool workers(w);

  for (int i = 0; i < n; ++i) {
    auto const worker_func = [m, &themap_manager, &themap]() {
      for (int i = 0; i < m; ++i) {
        themap_manager.dispatch([&themap, i]() { ++themap[i]; });
      }
    };

    workers.dispatch(worker_func);
  }

  }
  // The thread_pool destructor causes worker threads to be joined.
  // Therefore, themap is now free

  for (int i = 0; i < m; ++i) {
    // n tasks incremented each element by 1
    cpl::util::verify(n == themap[i], "error in thread_pool test");
  }
}



} // anonymous namespace


// int main( int const argc , char const * const * const argv ) {
int main() {

  try {
    test_dispatch();
    test_dispatch_n(std::cout, 1  , 100, 10000);
    test_dispatch_n(std::cout, 3  , 100, 10000);
    test_dispatch_n(std::cout, 10 , 100, 10000);
    test_dispatch_n(std::cout, 100, 100, 10000);
    
  } // global try
  catch( std::exception const& e )
  { std::cerr << e.what() << '\n' ; return 1 ; }

  return 0 ;
}
