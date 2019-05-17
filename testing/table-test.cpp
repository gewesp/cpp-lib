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

#include "cpp-lib/registry.h"
#include "cpp-lib/interpolation.h"
#include "cpp-lib/util.h"


using namespace cpl::util ;
using namespace cpl::math ;


int main() {

  try {

  lexer lex( std::cin , "stdin" ) ;
  parser p( lex , comma_optional ) ;

  while ( cpl::util::END != lex.peek_token()) {

    std::any a ;
    p.parse_term( a ) ;

    std::vector< std::any > const& va = 
      convert< std::vector< std::any > >( a ) ;

    table< double > t ;

    convert( va , t ) ;

    std::cout << t.dimension() << "-dimensional table, sizes:\n" ;
    std::copy( 
      t.size().begin() , 
      t.size().end  () ,
      std::ostream_iterator< std::size_t >( std::cout , " " ) 
    ) ;

    std::cout << '\n' ;

  } // end while(!END)

  } catch( std::exception const& e ) 
  { die( e.what() ) ; }

}
