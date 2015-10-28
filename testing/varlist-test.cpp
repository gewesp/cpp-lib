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
#include <exception>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <string>

#include "cpp-lib/varlist.h"
#include "cpp-lib/util.h"


using namespace cpl::util ;


int main() {
  
  try {

  std::vector< double > xs( 2 , 4711 ) ;
  long y = 815 ;

  varlist l ;
  l.vector_bind( "xs_" , xs ) ;
  l.bind( "y" , y ) ;

  std::cout << "creating xs_0 and xs_1 reference\n" ;

  double& xs_0 = l.reference< double >( "xs_0" ) ;
  double& xs_1 = l.reference< double >( "xs_1" ) ;

  xs[ 1 ] = 4712 ;

  always_assert( 4711 == xs_0 ) ;
  always_assert( 4712 == xs_1 ) ;

  std::vector< std::string > names ;
  names.push_back( "y"    ) ;
  names.push_back( "xs_1" ) ;

  std::cout << "creating stream_serializer with y and xs_1\n" ;
  stream_serializer ss( l , names ) ;
  std::ostringstream os ;
  os << ss ;
  always_assert( "815 4712" == os.str() ) ;

  std::istringstream is( std::string( "  -3 -8 " ) ) ;
  is >> ss ;
  always_assert( -3 == y       ) ;
  always_assert( -8 == xs[ 1 ] ) ;

  try {

    // reference<>() should throw a logic_error on wrong type.
    l.reference< int >( "xs_1" ) ;
    throw std::runtime_error( "varlist implementation not typesafe?" ) ;

  } catch( std::logic_error const& ) {}

  std::cout << "varlist tests OK.\n" ;
    
  } catch( std::exception const& e ) 
  { std::cerr << e.what() << std::endl ; return 1 ; }

}
