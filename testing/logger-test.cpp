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

#include "cpp-lib/registry.h"
#include "cpp-lib/util.h"
#include "cpp-lib/sys/util.h"
#include "cpp-lib/sys/logger.h"

std::string const conf_file = "logger-test.conf" ;

using namespace cpl::util ;


//
// For safety, be sure that the logger outlives the variables it
// references.
//

Logger l( "12345" ) ;


int main() {

  try {

  registry reg ;

  reg.read_from( conf_file , c_comments , comma_optional ) ;

  double foo = 0.815 ;
  float  bar = 4711 ;
  double t   = 0 ;

  l.bind( "foo"  , &foo ) ;
  l.bind( "bar"  , &bar ) ;
  l.bind( "time" , &t   ) ;

  // Configure l with entries from registry, entries begin with
  // test_logger_ .
  configure( l , reg , "test_logger_" ) ;

  std::string const& dest_port =
      reg.get< std::string >( "test_logger_udp_port" ) ;

  std::cout 
	<< "Enter nonnegative dt's.  Check UDP output on port "
    << dest_port << "." << std::endl
    << "foo and bar should increase according to dfoo/dt = 1, dbar/dt = 1.\n" 
  ;

  double const dt = .3 ;
  cpl::util::sleep_scheduler ss( dt ) ;
  while( 1 ) {
    
    t = ss.wait_next() ;
    std::cout << "t foo bar: " << t << " " << foo << " " << bar << std::endl;
	foo += dt ;
	bar += dt ;
	l.log( t ) ;

  }

  } catch( std::exception const& e )
  { die( e.what() ) ; }

}
