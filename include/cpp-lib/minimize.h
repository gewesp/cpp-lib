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
// Component: MATH
//
// Two functions for multidimensional, nonconvex, unconstrained minimization:
//   - Carl Rasmussen's line-search based minimize algorithm (uses gradients).
//   - Nelder & Mead's downhill simplex method (no gradients required).
//

#ifndef CPP_LIB_MINIMIZE_H
#define CPP_LIB_MINIMIZE_H

#include <vector>
#include <limits>
#include <map>
#include <cmath>

#include "cpp-lib/util.h"
#include "cpp-lib/math-util.h"
#include "cpp-lib/matrix-wrapper.h"


namespace cpl {

namespace detail_ {

//
// Fool the compiler, hopefully avoiding a division by zero warning.  Used
// in the Inf assertions below.
//

inline double zero() { return 0 ; }

//
// Better be safe than sorry:  Assert some properties we need from the floating
// point subsystem.
//
// The existence of NaNs and infinity is not guaranteed by the standard, but
// in practice most systems have them.  It is theoretically possible to work
// without them, but code becomes more complicated and error-prone as a
// result.
//

inline void assert_numeric_traits() {

  using namespace cpl::math ;

  always_assert( std::numeric_limits< double >::has_quiet_NaN ) ;
  always_assert( std::numeric_limits< double >::has_infinity  ) ;

  double const inf = std::numeric_limits< double >::infinity () ;
  double const nan = std::numeric_limits< double >::quiet_NaN() ;

  always_assert( std::min( -inf ,  123. ) == -inf ) ;
  always_assert( std::max(  inf , -inf  ) ==  inf ) ;

  always_assert( isnan( std::max( nan , -inf )   ) ) ;
  always_assert( isnan( std::sqrt( -1. )         ) ) ;
  always_assert( isnan( -1 / zero() + 1 / zero() ) ) ;

  always_assert( isinf( -1 / zero() ) ) ;
  always_assert( -1 / zero() < 1 / zero() ) ;

}

// Previous of a bidirectional iterator.
template< typename iterator > iterator previous( iterator it ) {
  --it ;
  return it ;
}


} // namespace detail_


namespace math {

//
// This is a C++ version of Carl Edward Rasmussen's multidimensional
// minimization function.  The algorithm and most variable names used are
// mostly the same.  The function signature has been adapted to C++ and
// slightly changed.
//
// The original Matlab implementation can be found at:
//
//   http://www.kyb.tuebingen.mpg.de/bs/people/carl/code/minimize/
//
// From the web page:
//
//   The matlab function minimize.m finds a (local) minimum of a (nonlinear)
//   multivariate function. The user must supply a function which returns the
//   value and partial derivatives wrt all variables. The function uses
//   conjugate gradients and approximate linesearches based on polynomial
//   interpolation with Wolfe-Powel conditions.
//

//
// The following note accompanies the original Matlab implementation:
//
// (C) Copyright 1999 - 2006, Carl Edward Rasmussen
//
// Permission is granted for anyone to copy, use, or modify these
// programs and accompanying documents for purposes of research or
// education, provided this copyright notice is retained, and note is
// made of any changes that have been made.
//
// These programs and documents are distributed without any warranty,
// express or implied.  As the programs were written for research
// purposes only, they have not been tested to the degree that would be
// advisable in any important application.  All use of these programs is
// entirely at the user's own risk.
//
// (http://www.kyb.tuebingen.mpg.de/bs/people/carl/code/minimize/Copyright)
//

//
// Find a local minimumm of a differentiable multivariate function.
//
// Arguments:
//
// - X  (size D): Starting point.
//
// - f          : Function R^D -> R, computing value and gradient.
//   The expression
//
//     f.evaluate(x, fx, dfx);
//
//   must be well-formed and evaluate the objective function at x, return the
//   function value in fx and the gradient in dfx.  evaluate() may assume that
//   n_rows(dfx) == n_rows(x).
//
// - length     : Max number of linesearches (if > 0), or -(max
//                number of evaluations of f) (if < 0).
//                Must be != 0.
//
// - red        : Reduction parameter, 1 by default.
//
// Return parameters, filled if the pointers passed are != NULL:
//
// - *n_ls      : Number of line searches performed.
//
// - *n_fe      : Number of evaluations of f performed plus 1.
//                (for compatibility with the Matlab version).
//
// - *fX        : Vector of function values computed.
//
// Return value:  The location of the approximate minimum.
//
// Detailed implementation notes can be found here:
//
//   http://www.kyb.tuebingen.mpg.de/bs/people/carl/code/minimize/minimize.m
//
// In particular, note that if the function terminates within a few iterations,
// it could be an indication that the function values and derivatives computed
// by f are inconsistent.
//
// You can use numerical_gradient() from optimization.h to crosscheck the
// gradients computed by your f function versus their numerical approximation.
//

template<typename F>
cpl::matrix::vector_t minimize(
  cpl::matrix::vector_t            X          ,
  F                      const& f          ,
  long                   const  length     ,
  double                 const  red    = 1 ,
  long                 * const  n_ls   = NULL ,
  long                 * const  n_fe   = NULL ,
  std::vector< double >* const  fX     = NULL
) {

  using namespace cpl::math ;

  cpl::detail_::assert_numeric_traits() ;

  always_assert( length != 0 ) ;
  always_assert( X == X ) ;
  always_assert( red == red ) ;

  // Algorithm parameters
  const double INT   = 0.1  ;
  const double EXT   = 3.0  ;
  const double RATIO = 10   ;
  const double SIG   = 0.1  ;
  const double RHO   = SIG/2;
  const long   MAX   = 20   ;

  double d0 = 0 ;
  double d1 = 0 ;
  double d2 = 0 ;
  double d3 = 0 ;
  double d4 = 0 ;

  double f0 = 0 ;
  double f1 = 0 ;
  double f2 = 0 ;
  double f3 = 0 ;
  double f4 = 0 ;

  double x1 = 0 ;
  double x2 = 0 ;
  double x3 = 0 ;
  double x4 = 0 ;

  double F0 = 0 ;
  double A  = 0 ;
  double B  = 0 ;

  cpl::matrix::vector_t X0  ;
  cpl::matrix::vector_t dF0 ;
  cpl::matrix::vector_t s   ;

  cpl::matrix::vector_t df0( cpl::matrix::n_rows(X) ) ;
  cpl::matrix::vector_t df3( cpl::matrix::n_rows(X) ) ;

  long i     = 0 ;
  long n_ls_ = 0 ;
  long n_fe_ = 0 ;
  long M     = 0 ;

  // const std::string S = length > 0 ? "Linesearch" : "Function evaluations";

  if (fX) { fX->clear() ; }

  bool ls_failed = false;
  f.evaluate(X, f0, df0);
  if (fX) { fX->push_back(f0); }

  // debug_output( std::cout , f0 ) ;

  if (length < 0) {
    ++i;
  }
  ++n_fe_ ;

  s = -df0;
  d0 = -(s|s);
  x3 = red/(1-d0);

  // std::cout.precision( 16 ) ;
  // debug_output( std::cout , transpose(X) ) ;
  while (i < std::abs(length)) {
    // std::cout << "i = " << i << std::endl;
    if (length > 0) {
      ++i;
    }
    ++n_ls_ ;

    X0 = X;
    F0 = f0;
    dF0 = df0;
    if (length > 0) {
      M = MAX;
    } else {
      M = std::min(MAX, -length-i);
    }


    while (1) {
      // std::cout << "while(1)\n" ;
      x2 = 0; f2 = f0; d2 = d0; f3 = f0; df3 = df0;
      bool success = false;
      while (!success && M > 0) {
        --M;
        if (length < 0) {
          ++i;
        }
        ++n_fe_ ;
        f.evaluate(X + x3 * s, f3, df3);
        if (isnan(f3) || isinf(f3) || cpl::matrix::isnan_any(df3) 
                                   || cpl::matrix::isinf_any(df3)) {
          x3 = (x2+x3)/2;
        } else {
          // FIXME:  this Success logic can be simplified.
          success = true;
        }
      }
      if (f3 < F0) {
        X0 = X+x3*s; F0 = f3; dF0 = df3;
      }
      d3 = df3 | s;
      if (d3 > SIG*d0 || f3 > f0+x3*RHO*d0 || M == 0) {
        break;
      }
      x1 = x2; f1 = f2; d1 = d2;
      x2 = x3; f2 = f3; d2 = d3;
      A = 6*(f1-f2)+3*(d2+d1)*(x2-x1);
      B = 3*(f2-f1)-(2*d1+d2)*(x2-x1);
      x3 = x1-d1*square(x2-x1)/(B+std::sqrt(B*B-A*d1*(x2-x1)));
      if (isnan(x3) || isinf(x3) || x3 < 0) {
        x3 = x2*EXT;
      } else if (x3 > x2*EXT) {
        x3 = x2*EXT;
      } else if (x3 < x2+INT*(x2-x1)) {
        x3 = x2+INT*(x2-x1);
      }
    }

    while ((std::fabs(d3) > -SIG*d0 || f3 > f0+x3*RHO*d0) && M > 0) {
      // std::cout << "while(|d3| > -SIG d0 || ... )\n" ;
      if (d3 > 0 || f3 > f0+x3*RHO*d0) {
        x4 = x3; f4 = f3; d4 = d3;
      } else {
        x2 = x3; f2 = f3; d2 = d3;
      }
      if (f4 > f0) {
        x3 = x2-(0.5*d2*square(x4-x2))/(f4-f2-d2*(x4-x2));
      } else {
        A = 6*(f2-f4)/(x4-x2)+3*(d4+d2);
        B = 3*(f4-f2)-(2*d2+d4)*(x4-x2);
        x3 = x2+(std::sqrt(B*B-A*d2*square(x4-x2))-B)/A;
      }
      if (isnan(x3) || isinf(x3)) {
        x3 = (x2+x4)/2;
      }
      x3 = std::max(std::min(x3, x4-INT*(x4-x2)),x2+INT*(x4-x2));
      f.evaluate(X + x3 * s, f3, df3);
      if (f3 < F0) {
        X0 = X+x3*s; F0 = f3; dF0 = df3;
      }
      --M;
      if (length < 0) {
        ++i;
      }
      ++n_fe_;
      d3 = df3|s;
    }

    // std::cout << "before if(|d3| < ...)\n" ;
    if (std::fabs(d3) < -SIG*d0 && f3 < f0+x3*RHO*d0) {
      X += x3*s; f0 = f3;
      if (fX) { fX->push_back(f0); }
      // This is always printed in the Matlab version, but we don't want it
      // in a general-purpose library.
      // std::cout << S << " " << i << "; Value " << f0 << std::endl;
      s *= ((df3|df3)-(df0|df3))/(df0|df0);
      s -= df3;
      df0 = df3;
      d3 = d0; d0 = (df0|s);
      if (d0 > 0) {
        s = -df0; d0 = -(s|s);
      }
      assert(d0 <= 0);
      x3 = x3 * std::min(RATIO, d3/(d0-std::numeric_limits<double>::min()));
      ls_failed = false;
    } else {
      X = X0; f0 = F0; df0 = dF0;
      if (ls_failed || i > std::abs(length)) {
        break;
      }
      s = -df0; d0 = -(s|s);
      x3 = 1/(1-d0);
      ls_failed = true;
    }
  }
  if (n_ls) { *n_ls = n_ls_ ; }
  if (n_fe) { *n_fe = n_fe_ ; }
  if (length > 0) { always_assert(n_ls_ <=  length) ; }
  if (length < 0) { always_assert(n_fe_ <= -length) ; }
  if (length > 0 && n_ls) { always_assert(i == *n_ls); }
  if (length < 0 && n_fe) { always_assert(i == *n_fe); }
  return X ;
}


//
// Implement Nelder & Mead's downhill-simplex algorithm [1] to find a minimum
// of a function R^d -> R.
//
// Parameters:
//
// - X:  a vector of n + 1 n-dimensional vectors for the initial simplex.
//
// - f:  The objective function.  The expression
//
//       double fx = f.evaluate(x);
//
//     must be well-formed for a vector_t x and compute the function value at
//     x.
//
//     The expression
//
//       double d = f.distance(x, y);
//
//     must be well-formed for vector_t x and y and is used in the convergence
//     criterion.
//
// - precision_argument, precision_value:
//               Return as soon as the difference between the best and the
//               worst function values is <= precision_value and
//               f.difference(x, y) <= precision_argument for the corresponding
//               locations.
//
// - maxiter:    Execute as much as that many iterations.
//
// - alpha:      Reflection parameter.
//
// - gamma:      Expansion parameter.
//
// - beta:       Contraction parameter.
//
// - verbose:    If >= 1, output statistics at the end of the optimization.
//               If >= 2, output information in each iteration.
//               Output goes to stdout.
//
// Return value:  The location of the approximate minimum.
//
// Note:  There can be as many as O(n*maxiter) function evaluations.
//

template< typename F >
cpl::matrix::vector_t downhill_simplex(
  std::vector< cpl::matrix::vector_t >  const& X                         ,
  F                                  const& f                         ,
  double                             const  precision_argument = 1e-8 ,
  double                             const  precision_value    = 1e-8 ,
  long                               const  maxiter            = 1000 ,
  double                             const  alpha              = 1    ,
  double                             const  beta               = .5   ,
  double                             const  gamma              = 2    ,
  int                                const  verbose            = 0
) {

  if( verbose >= 1 ) {
    std::cout << "downhill_simplex: " << X.size() - 1 << " dimensions."
              << std::endl ;
  }

  // Counters for the algorithm steps.
  long n_replace    = 0 ;
  long n_expand     = 0 ;
  long n_contract   = 0 ;
  long n_evaluate   = 0 ;
  long n_iterations = 0 ;

  // Variables:
  // n ... problem dimension
  // x0 ... average of best n.
  // xr ... reflected point.
  // xe ... expanded point.
  //
  // fxr, fxe, etc. the respective function values.
  //
  // fx_best, fx_worst ... current best and worst function value in simplex.
  //
  // xi, one_minus_beta_x1, xb: Temporary values.

  using cpl::matrix::vector_t ;
  using cpl::detail_::previous ;

  always_assert( precision_value  > 0   ) ;
  always_assert( precision_argument > 0 ) ;
  always_assert( maxiter >= 1           ) ;
  always_assert( alpha > 0              ) ;
  always_assert( gamma > alpha          ) ;
  always_assert( beta > 0               ) ;
  always_assert( beta < 1               ) ;

  always_assert( X.size() >= 1 ) ;
  const long n = cpl::matrix::n_rows(X[ 0 ]);
  always_assert( n + 1 == static_cast< long >( X.size() ) ) ;
  always_assert( n >= 1 ) ;
  for( long i = 0 ; i < n + 1 ; ++i ) {
    always_assert( cpl::matrix::n_rows(X[ i ]) == n ) ;
  }

  cpl::matrix::vector_t x0( n ) ;
  cpl::matrix::vector_t xr( n ) ;
  cpl::matrix::vector_t xe( n ) ;
  cpl::matrix::vector_t xb( n ) ;
  cpl::matrix::vector_t xi( n ) ;

  typedef std::multimap < double , cpl::matrix::vector_t > val_arg_map ;
  val_arg_map simplex ;

  for( long i = 0 ; i < n + 1 ; ++i ) {
    simplex.insert( std::make_pair( f.evaluate( X[ i ] ) , X[ i ] ) ) ;
    ++n_evaluate ;
  }

  for( ; n_iterations < maxiter ; ++n_iterations ) {

    always_assert( n + 1 == static_cast< long >( simplex.size() ) ) ;

    // Iterators to x_{n+1} and x_n
    const val_arg_map::      iterator p_xnp1 = previous( simplex.end() ) ;
    const val_arg_map::const_iterator p_xn   = previous( p_xnp1        ) ;

#if 0
    // This is the convergence criterion from the Wolfram description.  Doesn't
    // really work so well in practice since the best x can simply stay the
    // same.  Need to get a more detailed description in order to implement
    // this.  Need to reintroduce the related variables in that case.
    if( best_updated ) {

      if(    std::fabs( simplex.begin()->first - fx_best_old )
             <= precision_domain
          && norm_2( simplex.begin()->second - x_best_old )
             <= precision_value ) {
        break ;
      }

      best_updated = false ;

    }

     x_best_old = simplex.begin()->second ;
    fx_best_old = simplex.begin()->first  ;
#else
    static_cast<void>(p_xn);
#endif

    // Another idea for bailout:  Compare worst and best values.
    const double fx_best  = simplex.begin()->first ;
    const double fx_worst = p_xnp1         ->first ;

    cpl::matrix::vector_t const& x_best  = simplex.begin()->second ;
    cpl::matrix::vector_t const& x_worst = p_xnp1         ->second ;

    always_assert( fx_best <= fx_worst ) ;

    if( fx_worst - fx_best <= precision_value
        && f.distance( x_worst , x_best ) <= precision_argument ) {
      break ;
    }

    // Compute x0 = (x1 + ... + xn) / n
    // TODO:  This doesn't need to be recomputed each time.  It
    // should be updated depending on whether replacement,
    // expansion, or contraction occurs.
    x0.fill( 0 ) ;
    for( val_arg_map::const_iterator i = simplex.begin() ;
         i != p_xnp1 ;
         ++i ) {
      x0 += i->second ;
    }
    x0 *= 1. / n ;

    xr = x0 + alpha * ( x0 - p_xnp1->second ) ;
    double fxr = f.evaluate( xr ) ;
    ++n_evaluate ;
    double fxe = 0 ;

    // Improvement at all?
    if( fxr < p_xnp1->first ) {

      // New best value?  Then expand, see if we can get even better.
      if( fxr < simplex.begin()->first ) {

        xe = x0 + gamma * ( x0 - p_xnp1->second ) ;
        fxe = f.evaluate( xe ) ;
        ++n_evaluate ;
        if( fxe < fxr ) {
          std::swap( fxe , fxr ) ;
          std::swap(  xe ,  xr ) ;
        }
        assert( fxr < simplex.begin()->first ) ;
        ++n_expand ;
        if( verbose >= 2 ) {
          std::cout << "expand" << std::endl ;
        }
        simplex.erase( p_xnp1 ) ;
        // We know that this is going at the top, insert with hint.
        simplex.insert( simplex.begin() , std::make_pair( fxr , xr ) ) ;

      } else {

        ++n_replace ;
        if( verbose >= 2 ) {
          std::cout << "replace" << std::endl ;
        }
        simplex.erase( p_xnp1 ) ;
        // We don't know where it's going, no hint.
        simplex.insert( std::make_pair( fxr , xr ) ) ;

      }

    } else {

      ++n_contract ;
      if( verbose >= 2 ) {
        std::cout << "contract" << std::endl ;
      }
      always_assert( n + 1 == static_cast< long >( simplex.size() ) ) ;
      // Contract by beta around x1.
      val_arg_map simplex_new ;
      val_arg_map::const_iterator it = simplex.begin() ;
      simplex_new.insert( *it ) ;
      xb = ( 1 - beta ) * it->second ;
      while( ++it != simplex.end() ) {
        // x1 + beta( xi_old - x1 )
        assert( cpl::matrix::n_rows(xb) == n ) ;
        assert( cpl::matrix::n_rows(it->second) == n ) ;
        xi = xb + beta * it->second ;
        simplex_new.insert(
          std::make_pair( f.evaluate( xi ) , xi )
        ) ;
        ++n_evaluate ;
      }
      std::swap( simplex , simplex_new ) ;
      always_assert( n + 1 == static_cast< long >( simplex.size() ) ) ;

    }

  }

  always_assert( n_iterations == n_replace + n_expand + n_contract ) ;
  always_assert(
    n_evaluate == n + 1 + n_replace + 2 * n_expand + ( n + 1 ) * n_contract
  ) ;

  if( verbose >= 1 ) {
    std::cout
      << "iterations:           " << n_iterations << std::endl
      << "function evaluations: " << n_evaluate   << std::endl
      << "replace steps:        " << n_replace    << std::endl
      << "expansion steps:      " << n_expand     << std::endl
      << "contraction steps:    " << n_contract   << std::endl
    ;
  }

  return simplex.begin()->second ;

}

} // namespace math

} // namespace cpl

#endif // CPP_LIB_MINIMIZE_H
