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

#include <iostream>
#include <stdexcept>
#include <exception>


#include "cpp-lib/dispatch.h"


namespace {

void test_dispatch() {
  cpl::dispatch::dispatch_queue disq;

  for (int i = 0; i < 50; ++i) {
    disq.dispatch_sync([i]{ std::cout << i << std::endl; });
  }
}

} // anonymous namespace


// int main( int const argc , char const * const * const argv ) {
int main() {

  try {
    test_dispatch();
  } // global try
  catch( std::runtime_error const& e )
  { std::cerr << e.what() << '\n' ; return 1 ; }

  return 0 ;
}
