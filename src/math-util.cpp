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
#include <algorithm>
#include <vector>

#include "cpp-lib/math-util.h"

using namespace cpl::math ;


double cpl::math::sinc(double const x) {
  if (std::fabs(x) < std::numeric_limits<double>::epsilon()) {
    return std::cos(x);
  } else {
    return std::sin(x) / x;
  }
}

// Series:  cosc(x) = -x/2! + x^3/4! - x^5/6! +- ...
// TODO: Thorough mathematical analysis...
double cpl::math::cosc(double const x) {
  double const x2 = x*x;
  double const x3 = x * x2;
  double const x5 = x3 * x2;

  // Use the series if |x^3| < epsilon
  if (std::fabs(x3) < std::numeric_limits<double>::epsilon()) {
    return  -0.5                                 * x
           + 0.041666666666666666666666666666666 * x3
           - 0.001388888888888888888888888888888 * x5;
  } else {
    // x^3 > epsilon -> x > 3rd root of epsilon.  Should be fine (?)
    return (std::cos(x) - 1) / x;
  }
}


int cpl::math::log_2( const long x ) {

  assert( x > 0 ) ;

  long p = 1 , log = 0 ;

  while( 2 * p <= x ) { p *= 2 ; ++log ; }

  assert( 1 << log == p ) ;
  assert( p <= x ) ;
  assert( x < 2 * p ) ;

  return log ;

}


long cpl::math::next_power_of_two( const long x ) {

  if( x == 0 ) return 1 ;

  assert( x > 0 ) ;

  long y = 1 << cpl::math::log_2( x ) ;

  if( y < x ) y *= 2 ;
  assert( x <= y ) ;
  assert( y < 2 * x ) ;

  return y ;

}


long cpl::math::bit_reversal( const long x , const int l ) {

  assert( l >= 1 ) ;
  assert( 0 <= x ) ;
  assert( x < ( 1 << l ) ) ;

  long mask_x = 1 ,
       mask_y = 1 << ( l - 1 ) ;

  long y = 0 ;

  do {

    if( x & mask_x ) y |= mask_y ;
    mask_x <<= 1 ;
    mask_y >>= 1 ;

  } while( mask_y ) ;

  assert( 0 <= y ) ;
  assert( y < ( 1 << l ) ) ;

  return y ;

}

double cpl::math::discriminant
( double const& c , double const& b , double const& a )
{ return b * b - 4 * a * c ; }

cpl::math::quadratic_solver::quadratic_solver
( double const& c , double const& b , double const& a ) :
  D ( discriminant( c , b , a )                   ) ,
  x1( a == 0 ? - c / b : ( - b + std::sqrt( D ) ) / ( 2 * a ) ) ,
  x2( a == 0 ? - c / b : ( - b - std::sqrt( D ) ) / ( 2 * a ) )
{}


bool cpl::math::approximate_equal
( double const& x1 , double const& x2 , double const& f ) {

  if( ( x1 <  0 && x2 >  0 ) || ( x1 > 0 && x2 < 0 ) ) { return false ; }
  if(   x1 == 0 && x2 == 0                           ) { return true  ; }
  if(   x1 == 0 || x2 == 0                           ) { return false ; }
  if(    x1 / x2 > 1 + f * std::numeric_limits< double >::epsilon()
      || x2 / x1 > 1 + f * std::numeric_limits< double >::epsilon() )
  { return false ; }

  return true ;

}
