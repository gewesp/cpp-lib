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

//
// DO NOT DIRECTLY INCLUDE FROM APPLICATION CODE.
//

//
// Realtime stuff implemented on top of POSIX 1b (Solaris, Linux,
// QNX,...).
//

//
// Somebody already seems to define this...
//
// #define _POSIX_C_SOURCE 199506
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>


namespace cpl {

namespace util {

//
// Schedule periodic events with fixed time delta.  Bound to some
// POSIX realtime clock.
//

struct realtime_scheduler {

  // Construct with given time delta dt [s].
  realtime_scheduler( double const& dt ) ;

  ~realtime_scheduler() ;

  // Wait (sleep) until next slice and return current time [s].  Waiting is
  // not cumulative, if you miss a time slice, it's lost.
  double wait_next() ;

  // Get current time [s].
  double time() ;

private:

  :: timer_t timer ;  // Can't use auto_resource here because no invalid 
                      // timer value defined.
  ::sigset_t sigs  ;

} ;


} // namespace util

} // namespace cpl
