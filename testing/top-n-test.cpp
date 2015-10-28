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
#include <list>
#include <vector>

#include <cstdlib>

#include "cpp-lib/util.h"
#include "cpp-lib/top-n.h"

using namespace cpl::util ;

template< int N > void test_top_n(int const n, long const mod = 1000000000) {
  std::vector< int > v ;
  cpl::util::top_n< int , N > tn ;

  for ( int i = 0 ; i < n ; ++i ) {
    always_assert( tn.size() == std::min( tn.capacity() , i ) ) ;

    int const r = std::rand() % mod;
    v.push_back(r);
    tn.push(r);
  }

  always_assert( tn.size() == std::min( tn.capacity() , n ) ) ;

  std::sort( v.begin() , v.end() ) ;

  // Verify the first n elements are equal
  int i = 0;
  for ( auto const elt : tn ) {
    always_assert( elt == v[i] ) ;
    ++i ;
  }

}

int main() {

  std::srand( 12345 ) ;
  test_top_n< 1 >( 0 );
  test_top_n< 1 >( 1 );
  test_top_n< 1 >( 100 );

  test_top_n< 100 >( 0 );
  test_top_n< 100 >( 1 );
  test_top_n< 100 >( 100 );
  test_top_n< 100 >( 1000 );
  
  test_top_n< 5 >( 0 );
  test_top_n< 5 >( 1 );
  test_top_n< 5 >( 100 );
  test_top_n< 5 >( 1000 );

  test_top_n< 5 >( 0 , 100 );
  test_top_n< 5 >( 1 , 100 );
  test_top_n< 5 >( 100 , 100 );
  test_top_n< 5 >( 1000 , 100 );

  test_top_n< 5 >( 0 , 10 );
  test_top_n< 5 >( 1 , 10 );
  test_top_n< 5 >( 100 , 1 );
  test_top_n< 5 >( 1000 , 1 );
}
