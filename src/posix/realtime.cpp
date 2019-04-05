//
// Copyright 2019 and onwards by KISS Technologies GmbH, Switzerland
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

// This is .h, not .hpp like everything else in boost.  WTF.
#include "boost/predef.h"

// This is only supported on Linux.
#if (BOOST_OS_LINUX)
#  include "cpp-lib/linux/realtime.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>

#include <cmath>
#include <cstring>
#include <cerrno>

#include <sys/time.h>
#include <unistd.h>

#include "cpp-lib/posix/wrappers.h"
#include "cpp-lib/sys/realtime.h"
  
using namespace cpl::detail_ ;


namespace {

int const MY_TIMER  = ITIMER_REAL ;
int const MY_SIGNAL = SIGALRM     ;


// Wait for signal s, discarding any information.

inline void wait4( ::sigset_t const& ss ) {
	
  int dummy ;
  STRERROR_CHECK( ::sigwait( &ss , &dummy ) ) ;

}


} // anonymous namespace


cpl::util::realtime_scheduler::realtime_scheduler( double const& dt ) {

  always_assert( dt > 0 ) ;

  sigs = block_signal( MY_SIGNAL ) ;

  // Set up timer
  
  ::itimerval itv ;
  itv.it_value    = to_timeval( 1e-6 ) ; // arm: need nonzero value
  itv.it_interval = to_timeval( dt   ) ;

  STRERROR_CHECK
  ( ::setitimer( MY_TIMER , &itv , 0 ) ) ;

}


double cpl::util::realtime_scheduler::time() {

  ::timeval t ;
  ::gettimeofday( &t , 0 ) ;

  return to_double( t ) ;

}
    

double cpl::util::realtime_scheduler::wait_next() {

  wait4( sigs ) ;
  return time() ;

}


cpl::util::realtime_scheduler::~realtime_scheduler() { 
  
  // Disable our timer.
  ::itimerval itv ;
  itv.it_value    = to_timeval( 0 ) ;
  itv.it_interval = to_timeval( 0 ) ;

  STRERROR_CHECK
  ( ::setitimer( MY_TIMER , &itv , 0 ) ) ;

}

#else
#  warning "Realtime functions not supported on this operating system platform"
#  warning "You can safely ignore this warning provided you don't include"
#  warning "cpp-lib/realtime.h in your project."
#endif
