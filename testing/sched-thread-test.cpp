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

#include "boost/predef.h"


// Compile a null program if we're on anything else than Linux
#if (BOOST_OS_LINUX)

#include <iostream>
#include <string>
#include <thread>

#include <cassert>

#include <boost/lexical_cast.hpp>

#include "cpp-lib/util.h"
#include "cpp-lib/sys/util.h"
#include "cpp-lib/sys/realtime.h"

using namespace cpl::util ;


void usage() { 
  std::cerr << 
"usage: sched-thread-test <dt_cout> <dt_cerr>\n" 
"This will instantiate a realtime_scheduler with interval output on stdout,\n"
"and a loop with a sleep and interval output on stderr in a separate thread.\n"
;
}


struct cerr_thread {

  cerr_thread( double const& dt ) : dt( dt ) {}

  void operator()() ;
  
private:

  double dt ;

} ;
  

void cerr_thread::operator()() {

  // Sleep some arbitrary amount to avoid cout/cerr clash on program
  // startup.
  cpl::util::sleep( .0123456 ) ;
  std::cerr.precision( 16 ) ;

  while( 1 ) { 
	
    std::cerr << "cerr: t = " << cpl::util::time() << std::endl ; 
    cpl::util::sleep( dt ) ;
  
  }

}



int main( int const argc , char const * const * const argv ) {

  if( argc != 3 ) { usage() ; return 1 ; }

  try {

  double const dt1 = boost::lexical_cast< double >( argv[ 1 ] ) ;
  double const dt2 = boost::lexical_cast< double >( argv[ 2 ] ) ;

  if( dt1 <= 0 || dt2 <= 0 ) { usage() ; return 1 ; }


  //
  // Create the scheduler *before* the thread to have it inherit the
  // blocked signal.
  //

  realtime_scheduler s( dt1 ) ;
  
  // The following would be parsed as a function declaration:
  // std::thread t(cerr_thread(dt2));
  std::thread t((cerr_thread(dt2)));
  t.detach();

  std::cout.precision( 16 ) ;

  while( 1 ) { 
    const double t = s.wait_next() ;
    std::cout << "cout: t = " << t << std::endl ;
  }

  } catch( std::exception const& e ) 
  { std::cerr << e.what() << std::endl ; return 1 ; }

}

#else

// Empty main().  Realtime functions not supported on this OS.
int main() {}

#endif
