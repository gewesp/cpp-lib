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
#include <iterator>
#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <memory>

#include <cassert>


#include "cpp-lib/sys/network.h"
#include "cpp-lib/util.h"

using namespace cpl::util::network ;

std::string const port = "4711" ;


void usage( std::string const& name ) {

  std::cerr << 
    "usage: " << name << " local port\n"
    "usage: " << name << " remote host remote port\n"
  ;

}


int main( int argc , char const* const* const argv ) {

  try {

  if( 2 == argc ) {

    stream_address a( argv[ 1 ] ) ;

    for( unsigned i = 0 ; i < a.size() ; ++i ) {

      std::cout << name( a.at( i ) ) << '\n' ;

    }

  } else if( 3 == argc ) {
    
    node_service const ns( argv[ 1 ] , argv[ 2 ] ) ;
    stream_address const a( ns ) ;

    for( unsigned i = 0 ; i < a.size() ; ++i ) {

      std::cout << name( a.at( i ) ) << '\n' ;

    }

  } else {

    usage( argv[ 0 ] ) ;
    return 1 ;

  }

  } // end global try

  catch( std::exception const& e ) 
  { cpl::util::die( e.what() ) ; }

}
