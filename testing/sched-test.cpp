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

#include <cassert>
#include <cstdlib>

#include <boost/lexical_cast.hpp>

#include "cpp-lib/sys/realtime.h"
#include "cpp-lib/util.h"

using namespace cpl::util ;


void usage() 
{ std::cerr << "usage: sched-test <dt>\n" ; }


int main( int const argc , char const * const * const argv ) {
  
  try {

  if( argc != 2 ) { usage() ; return 1 ; }

  double const dt = boost::lexical_cast< double >( argv[ 1 ] ) ;

  if( dt <= 0 ) { usage() ; return 1 ; }


  // realtime_scheduler s( dt ) ;
  sleep_scheduler s( dt ) ;

  std::cout.precision( 16 ) ;

  while( 1 ) 
  { std::cout << s.wait_next() << std::endl ; }

  } catch( std::exception const& e ) 
  { std::cerr << e.what() << std::endl ; return 1 ; }

}
