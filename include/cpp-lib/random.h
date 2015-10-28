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
// Component: UNSUPPORTED
//
// TODO: Replace by C++11 <random> facilities
//


#ifndef CPP_LIB_RANDOM_H
#define CPP_LIB_RANDOM_H

#include <fstream>
#include <iterator>
#include <memory>
#include <streambuf>

#include <cstdlib>
#include <cmath>
#include <cassert>

//
// Some rudimentary Pseudo Random Number Generation.
//


namespace cpl {

namespace math {


struct system_rng {

  double operator()()
  { return std::rand() / ( RAND_MAX + 1. ) ; }

} ;


//
// A generator using Operating System functionality to generate typically
// high-quality random numbers.  Assumes the presence of a file /dev/urandom.
// If this file cannot be read, the constructor fails.
//
// The generator can return at most
//
//   std::numeric_limits< unsigned long >::max() + 1
//
// distinct values.
//

struct urandom_rng {

  urandom_rng() ;

  double operator()() ;

private:

  std::filebuf ib ;
  std::istreambuf_iterator< char > it ;

} ;


/// Return X ~ lambda * exp( -lambda x )

template< typename rng >
double exponential_distribution( rng& r , double const& lambda ) {

  assert( lambda > 0 ) ;
  double const u = 1 - r() ;
  assert( u >  0 ) ;
  assert( u <= 1 ) ;

  return -std::log( u ) / lambda ;

}



//
// Return
//
//   X = sigma * ( X_1 + ... + X_n - n / 2 ) / sqrt( n / 12 ),
//
// where X_i ~ U[ 0 , 1 ] and independent.
//
// Note that for U[0,1], sigma^2 = 1/12, hence for X_1 + ... + X_n it's n/12.
//
// By the Central Limit Theorem, X's distribution will approach
// N( 0 , sigma ) as \a n -> infinity.
//

template< typename rng >
double n_times_distribution( rng& r , double const& sigma , int const n ) {

  assert( n >= 1 ) ;

  double sum = 0 ;
  for( int i = 0 ; i < n ; ++i ) { sum += r() ; }

  return sigma * ( sum - n / 2. ) / std::sqrt( n / 12. ) ;

}

} // end namespace math

namespace util {

//
// Returns a random sequence whose size is drawn from sd and values from
// vd.
//

template<typename cont, typename rng, typename sizedist, typename valdist>
cont random_sequence(rng& rand, sizedist& sd, valdist& vd) {
  unsigned long const size = sd(rand);
  cont ret;

  // ret.reserve(size);

  for (unsigned long i = 0; i < size; ++i) {
    ret.push_back(vd(rand));
  }
  return ret;
}
    

} // namespace util

} // end namespace cpl


#endif // CPP_LIB_RANDOM_H
