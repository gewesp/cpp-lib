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

#include "cpp-lib/random.h"
#include "cpp-lib/util.h"

using namespace cpl::math ;

int const n = 10000 ;
int const factor = 4 ;
double const lambda = .1 ;
double const sigma = 1 ;


template< typename rng > void test() {

  rng r ;

  std::cout << "uniform:\n" ;
  for( int i = 0 ; i < n ; ++i )
  { std::cout << r() << '\n' ; }

  std::cout << "exponential:\n" ;
  for( int i = 0 ; i < n ; ++i )
  { std::cout << exponential_distribution( r , lambda ) << '\n' ; }

  std::cout << "n_times:\n" ;
  for( int i = 0 ; i < n ; ++i )
  { std::cout << n_times_distribution( r , sigma , factor ) << '\n' ; }

}

int main() {

  try {

    std::cout << "system_rng\n" ;
    test<  system_rng >() ;
    std::cout << "urandom_rng\n" ;
    test< urandom_rng >() ;

  } catch( std::runtime_error const& e )
  { cpl::util::die( e.what() ) ; }

}
