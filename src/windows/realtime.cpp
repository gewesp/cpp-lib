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
// This define necessary according to MSN documentation on
// CreateWaitableTimer().
//

#define _WIN32_WINNT 0x0400

#include <cmath>
#include <iostream>

#include "cpp-lib/util.h"
#include "cpp-lib/windows/wrappers.h"

#include "cpp-lib/sys/realtime.h"

using namespace cpl::util ;


cpl::util::realtime_scheduler::realtime_scheduler( double const& dt ) {

  always_assert( dt > 0 ) ;

  LARGE_INTEGER dueTime ;
  dueTime.QuadPart = -1 ;

  const LONG period = static_cast< LONG >( std::floor( dt * 1000 + 0.5 ) ) ;

  timer.reset( CreateWaitableTimer( 0 , 0 , 0 ) ) ;

  if( !timer.valid() )
  { cpl::detail_::last_error_exception( "CreateWaitableTimer()" ) ; }

  if( !SetWaitableTimer( timer.get() , &dueTime , period , 0 , 0 , 0 ) )
  { cpl::detail_::last_error_exception( "SetWaitableTimer()"    ) ; }

}


double cpl::util::realtime_scheduler::wait_next() {

  if( WAIT_OBJECT_0 != WaitForSingleObject( timer.get() , INFINITE ) )
  { cpl::detail_::last_error_exception( "WaitForSingleObject()" ) ; }

  return time() ;

}
