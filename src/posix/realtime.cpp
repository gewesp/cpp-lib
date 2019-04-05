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
// TODO: This is a mess. Clean up and make the public header independent
// of implementation (using Pimpl if needed).
//

// This is .h, not .hpp like everything else in boost.  WTF.
#include "boost/predef.h"

// Supported on Linux and MacOS.
// MacOS is based on POSIX.
// Linux has POSIX1B support for realtime, slightly more advanced.
#if (BOOST_OS_MACOS)
#  include "cpp-lib/posix/realtime.h"

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

#elif (BOOST_OS_LINUX)

#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>

#include <cmath>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cpp-lib/detail/platform_wrappers.h"
#include "cpp-lib/posix1b/realtime.h"
  
using namespace cpl::detail_ ;


namespace {

   ::clockid_t const MY_CLOCK = CLOCK_REALTIME  ; // Std. Posix
// ::clockid_t const MY_CLOCK = CLOCK_MONOTONIC ; // QNX
// ::clockid_t const MY_CLOCK = CLOCK_HIGHRES   ; // Solaris
// ::clockid_t const MY_CLOCK = CLOCK_SGI_CYCLE ;

// int const MY_SIGNAL = SIGUSR1 ;
int const MY_SIGNAL = SIGRTMIN ;


// Wait for signal s, discarding any information.

inline void wait4( ::sigset_t const& ss ) {
	
  siginfo_t dummy ;
  STRERROR_CHECK( ::sigwaitinfo( &ss , &dummy ) ) ;

}


} // anonymous namespace


cpl::util::realtime_scheduler::realtime_scheduler( double const& dt ) {

  always_assert( dt > 0 ) ;

  // Block MY_SIGNAL.  Will be handled by sigwaitinfo().

  sigs = block_signal( MY_SIGNAL ) ;

  // Set up timer
 
  ::sigevent my_event ;
  my_event.sigev_notify = SIGEV_SIGNAL ;
  my_event.sigev_signo  = MY_SIGNAL    ;
  // (union: alternative is sival_ptr)
  my_event.sigev_value.sival_int = 0            ;   

  STRERROR_CHECK
  ( ::timer_create( MY_CLOCK , &my_event , &timer ) ) ;
  
  ::itimerspec its ;
  its.it_value    = to_timespec( 1e-9 ) ; // arm: need nonzero value
  its.it_interval = to_timespec( dt   ) ;

  STRERROR_CHECK( ::timer_settime( timer , 0 , &its , 0 ) ) ;
  
} ;


double cpl::util::realtime_scheduler::time() {

  ::timespec t ;
  STRERROR_CHECK( ::clock_gettime( MY_CLOCK , &t ) ) ;

  return to_double( t ) ;

}
    

double cpl::util::realtime_scheduler::wait_next() {

  wait4( sigs ) ;
  return time() ;

}


cpl::util::realtime_scheduler::~realtime_scheduler()
{ ::timer_delete( timer ) ; }


// 
// Some RT stuff snippets, may be incorporated in the future...
//

#if 0
void check_rtsig() {

  errno = 0 ;
  long const i = ::sysconf( _SC_REALTIME_SIGNALS ) ;
  if( errno ) { die( "sysconf" ) ; }

  if( i <= 0 ) {
    std::cerr << "realtime signals not supported\n" ;
    std::exit( 1 ) ;
  }
  
}



void set_sched() {

  sched_param sp ;

  sp.sched_priority = sched_get_priority_max( SCHED_RR ) ;

  if( sched_setscheduler( 0 , SCHED_RR , &sp ) < 0 ) {
    
    std::cerr << "warning: couldn't set scheduling parameters\n" ;

  }

}


    timespec res ;
    if( ::clock_getres( MY_CLOCK , &res ) < 0 )
    { die( "clock_getres" ) ; }


#endif 

#else
#  warning "Realtime functions not supported on this operating system platform"
#  warning "You can safely ignore this warning provided you don't include"
#  warning "cpp-lib/realtime.h in your project."
#endif
