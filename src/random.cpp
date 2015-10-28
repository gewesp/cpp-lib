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

#include <iterator>
#include <iostream>
#include <streambuf>
#include <stdexcept>
#include <exception>
#include <string>
#include <limits>

#include "cpp-lib/random.h"
#include "cpp-lib/util.h"

using namespace cpl::util::file ;

namespace {

std::string const random_file = "/dev/urandom" ;

} // anonymous namespace


cpl::math::urandom_rng::urandom_rng()
: ib( open_readbuf( random_file ) ) ,
  it( std::istreambuf_iterator< char >( &ib ) )
{}

double cpl::math::urandom_rng::operator()() {

  unsigned long l ;

  unsigned char* const p = reinterpret_cast< unsigned char* >( &l ) ;

  // Assign sizeof( ulong ) bytes into l.
  for( unsigned i = 0 ; i < sizeof( unsigned long ) ; ++i ) {
    p[ i ] = *it ;
    ++it ;
  }

  if( it == std::istreambuf_iterator< char >() ) {
    throw std::runtime_error( "couldn't read from " + random_file ) ;
  }

  return
    l / static_cast< double >( std::numeric_limits< unsigned long >::max() ) ;

}
