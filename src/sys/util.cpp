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


#include <cmath>

#include "cpp-lib/sys/util.h"

#include "cpp-lib/platform/wrappers.h"


using namespace cpl::util ;


double cpl::util::sleep_scheduler::wait_next() {

  double const n = ( this->time() - t_0 ) * hz ;

  if( n > n_next ) {   n_next = std::ceil( n ) ; } // Missed a slice.
  else             { ++n_next ;                  }

  assert( n_next >= n ) ;

  cpl::util::sleep( dt * ( n_next - n ) ) ;

  return t_0 + dt * n_next ;

}
