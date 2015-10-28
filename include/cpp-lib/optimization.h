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
// TODO: De-inline
//

//
// Test functions for optimization routines.
//

#ifndef CPP_LIB_OPTIMIZATION_H
#define CPP_LIB_OPTIMIZATION_H

#include <map>

#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/math-util.h"
#include "cpp-lib/util.h"


namespace cpl {

namespace math {

namespace {

// The constant for the Rosenbrock function.
const double C = 100;

}

//
// Return the value
//
//   sum_{ i = 1 ... n - 1 } ( 1 - x_i )^2 + 100 * ( x_{i+1} - x_i^2 )^2
//
// This function is nonconvex and has a unique global minimum at x = (1, ...,
// 1).  For n >= 5, it has other local minima, though.
//
// See:  http://en.wikipedia.org/wiki/Rosenbrock_function
//

inline double rosenbrock( cpl::matrix::vector_t const& x ) {

  using cpl::math::square ;

  unsigned long const n = x.rows();

  always_assert( 1 == x.cols() ) ;
  always_assert( 2 <= n );

  double r = 0;

  for( unsigned long i = 0 ; i < n - 1 ; ++i ) {
    r += square( 1 - x(i) ) + C * square( x( i + 1 ) - square( x( i ) ) ) ;
  }

  return r ;

}


//
// Returns the exact gradient of rosenbrock(x) as a column vector.
//

inline cpl::matrix::vector_t rosenbrock_gradient( cpl::matrix::vector_t const& x ) {

  using cpl::math::square ;

  const unsigned long n = cpl::matrix::n_rows(x);

  always_assert( 1 == cpl::matrix::n_columns( x ) );
  always_assert( 2 <= n );

  cpl::matrix::vector_t dr( n );

  for( unsigned long i = 1 ; i < n - 1 ; ++i ) {

    dr( i ) = -2 * ( 1 - x( i ) )
            -  4 * C * x( i ) * ( x( i + 1 ) - square( x( i ) ) )
            +  2 * C * ( x( i ) - square( x( i - 1 ) ) )
    ;

  }

  dr( 0 ) = -2 * ( 1 - x( 0 ) )
          -  4 * C * x( 0 ) * ( x( 1 ) - square( x ( 0 ) ) )
  ;

  dr( n - 1 ) = 2 * C * ( x( n - 1 ) - square( x( n - 2 ) ) ) ;

  return dr ;

}

//
// Compute function value and gradient of the Rosenbrock function.
// Non-optimized and instrumented version, use only for testing and evaluation.
//

struct rosenbrock_f {

  rosenbrock_f( bool const log = false ) : n_eval( 0 ) , log( log ) {}

  void evaluate(
    cpl::matrix::vector_t const& x   ,
    double                  & fx  ,
    cpl::matrix::vector_t      & dfx
  ) const {
    ++n_eval ;
     fx = rosenbrock         ( x ) ;
    dfx = rosenbrock_gradient( x ) ;
    if( log ) {
      ++evals[ x ].first ;
      evals[ x ].second = n_eval ;
    }
  }

  // How often was evaluate called with a certain argument and when was the
  // last evaluation?
  typedef std::map< cpl::matrix::vector_t , std::pair< long , long > > argcnt_t ;

  argcnt_t mutable evals ;

  // Number of evaluate() calls.
  long mutable n_eval ;

private:

  // Log calls to evaluate?
  bool log ;

} ;


//
// Returns gradient of F @ x, computed numerically with evaluations
// at x +- h * e_i with canonical basis vectors e_i.
//

template< typename F >
cpl::matrix::vector_t numerical_gradient(
  F const& f ,
  cpl::matrix::vector_t const& x ,
  double const& h
) {

  always_assert( h > 0 ) ;

  unsigned long const n = x.rows() ;

  // Copy x so we can modify it.
  cpl::matrix::vector_t xx = x;
  cpl::matrix::vector_t dx( n ) ;

  for( unsigned long i = 0 ; i < n ; ++i ) {

    double const xi = xx( i ) ;
    xx( i ) = xi + h ; double const f1 = f( xx ) ;
    xx( i ) = xi - h ; double const f2 = f( xx ) ;
    xx( i ) = xi ;

    dx( i ) = ( f1 - f2 ) / ( 2 * h ) ;

  }

  return dx ;

}

} // namespace math

} // namespace cpl

#endif // CPP_LIB_OPTIMIZATION_H
