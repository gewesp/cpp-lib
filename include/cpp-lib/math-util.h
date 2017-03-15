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


#ifndef CPP_LIB_MATH_UTIL_H
#define CPP_LIB_MATH_UTIL_H

#include <algorithm>
#include <exception>
#include <functional>
#include <limits>
#include <stdexcept>
#include <sstream>
#include <vector>
// #include <iostream>

#include <cassert>
#include <cmath>

#include "cpp-lib/assert.h"
#include "cpp-lib/matrix-wrapper.h"

//
// These shouldn't be in C++, but some systems do declare them as macros.  They
// only are in C99, which is not part of C++.  We declare our own isinf and
// isnan functions below.
//

#undef isnan
#undef isinf

namespace cpl {

namespace detail_ {

//
// Helper class as a default argument to solver step() function.
//

template< typename M > struct noop_inspector_type {

  void operator()( M const& , double const& ) const {}

} ;


//
// ODE solver base class template.
//

template< typename M > struct ode_solver_base {

  ode_solver_base( M& m , double const& dt )
  : t ( 0  ) ,
    dt( dt ) ,
    m ( m  ) ,
    x ( m.default_state         () ) ,
    xd( m.default_discrete_state() )
  {}

  // Return current time.
  double const& time() const { return t ; }

  // Set current time.
  void set_time( double const& t_ ) { t = t_ ; }

  // Return time step.
  double const& time_step() const { return dt ; }

  // Return model.
  M const& model() const { return m ; }
  M      & model()       { return m ; }

  // Return current state.
  typename M::state_type const& state() const { return x ; }
  typename M::state_type      & state()       { return x ; }

  typename M::discrete_state_type const& discrete_state() const { return xd ; }
  typename M::discrete_state_type      & discrete_state()       { return xd ; }

  // An no-op inspector that can be used as an argument to step().
  noop_inspector_type< M > noop_inspector() const
  { return noop_inspector_type< M >() ; }

protected:

  // Time.
  double t ;

  // Time delta.
  const double dt ;

  // The model.
  M& m ;

  // Model continuous and discrete state.
  typename M::state_type          x  ;
  typename M::discrete_state_type xd ;

} ;

struct isnan_function_object {
  bool operator()(double const x) const {
    return x != x;
  }
};

} // namespace detail_


namespace math     {


/// Guess what...

double constexpr pi = 3.14159265358979323846264338327950288419716939937510 ;


/// Default precision (number of decimal digits) for writing to streams.
enum { DEFAULT_PRECISION = 16 } ;


//
// sin(x)/x and (cos(x) - 1) / x, sanitized around zero.
//

double sinc(double x);
double cosc(double x);


//
// A sane modulo function.
//

inline double modulo( double const& x , double const& m ) {

  assert( m > 0 ) ;

  double const y = std::fmod( x , m ) ;

  if( y < 0 ) { return y + m ; }
  else        { return y     ; }

}


//
// Return an equivalent angle in [ 0 , 2pi).
//

inline double angle_0_2pi( double const& a )
{ return modulo( a , 2 * pi ) ; }


/// \return The angle equivalent to a in [-pi , pi).
inline double angle_mpi_pi( double const& a ) {

  double const aa = angle_0_2pi( a ) ;
  if( aa > pi ) { return aa - 2 * pi ; }
  else          { return aa          ; }

}

/// \return The angle equivalent to a [degrees] in [-180 , 180).
inline double angle_m180_180( double const& a ) {
  double const m = std::fmod(a, 360.0);
  if (m < -180.0) { return m + 360.0; }
  if (m >  180.0) { return m - 360.0; }
  return m;
}

//
// Declare our own isnan() and isinf() functions.  They're *not* in C89, and
// hence also not in C++, even if some systems do define them.  Future versions
// of C++ might include them in which case they need to be removed here.
//

inline bool isnan( double const x ) {
  return x != x ;
}


inline bool isinf( double const x ) {
  return 0 == 1/x ;
}


// Sign function with optional threshold.

// Returns sign of x or 0 if |\a x| <= \a threshold.

template< typename T >
inline int sign( T const& x , T const& threshold = 0 ) {

  assert( threshold >= 0 ) ;

  if( x < -threshold ) return -1 ;
  if( x >  threshold ) return  1 ;
  return 0 ;

}


/// Maximum of three values.

/// \return max( x1 , x2 , x3 ).

// Microsoft Compiler Version 13.10.3077 for 80x86 dies with fatal error
// C1001: INTERNAL COMPILER ERROR on this...
#ifndef _MSC_VER
template< typename T >
T max( T const& x1 , T const& x2 , T const& x3 ) {

  return std::max( x1 , std::max( x2 , x3 ) ) ;

}
#endif


/// Clamp value to a range.

/// \reval x Is set to \a lower if \a x < \a lower or \a upper if \a x >
/// \a upper.  Otherwise, \a x is left alone.
/// \precond \a lower <= \a upper.

template< typename T >
inline void clamp( T& x , T const& lower , T const& upper ) {
  assert( lower <= upper ) ;
  if     ( x < lower ) { x = lower ; }
  else if( x > upper ) { x = upper ; }
}


//
// Return the relative error | ( a - b ) / b | for b != 0.
//

inline double relative_error( double const& a , double const& b ) {
  assert( b != 0 ) ;
  return std::fabs( ( a - b ) / b ) ;
}


//
// Approximate equality.  Returns true iff either x1 = 0 and x2 = 0, or
// x1 and x2 are nonzero, have the same sign and
//
//   | x1 / x2 | <= 1 + f eps and | x2 / x1 | <= 1 + f eps
//
// where eps = numeric_limits<double>::epsilon().
//

bool approximate_equal
( double const& x1 , double const& x2 , double const& f = 1 ) ;


/// Minimum of positive or undefined values.

/// \return min( \a x1 , \a x2 ), where negative values of the input
/// parameters stand for undefined values.  If one of the parameters
/// is negative, the other parameter is returned.

template< typename T >
T pos_min( T const& x1 , T const& x2 ) {

  if( x1 < 0 ) { return x2 ; }
  if( x2 < 0 ) { return x1 ; }

  assert( x1 >= 0 ) ;
  assert( x2 >= 0 ) ;

  return std::min( x1 , x2 ) ;

}


/// \return \a x if \a x is even, otherwise return \a x - 1.

template< typename T >
inline T even( T x ) {

  // assert( std::numeric_limits< T >::is_integer ) ;

  return x % 2 ? x - 1 : x ;

}


//
// Return a solution x s.t. f(x) ~ 0 for continuous f.  Searches in the
// interval [ x0 , x1 ].  Not very optimized and not yet mathematically
// verified.  The larger the e, the less accurate.
//

template< typename F >
double binary_subdivision(
  F const& f ,
  double const& x0 ,
  double const& x1 ,
  double const& e = 1 ) {

  double const xm = ( x0 + x1 ) / 2 ;
  if( approximate_equal( x0 , x1 , e ) ) { return xm ; }

  double const fx0 = f( x0 ) ;
  double const fx1 = f( x1 ) ;

  if( fx0 * fx1 >= 0 ) { return xm ; }

  double const fxm = f( xm ) ;

  // std::cout.precision(16);
  // std::cout << x0 << " " << x1 << std::endl;
  // std::cout << fx0 << " " << fx1 << std::endl;
  // std::cout << std::endl ;

  if( fxm * fx1 >= 0 ) { return binary_subdivision( f , x0 , xm ) ; }
  if( fx0 * fxm >= 0 ) { return binary_subdivision( f , xm , x1 ) ; }

  throw std::runtime_error( "binary_subdivision() couldn't find a solution" ) ;

}


//
// Computes two real solutions of the quadratic equation a x^2 + b^x + c = 0
// and stores them in x1 and x2:
//
//   x1 = ( -b + sqrt( b^2 - 4 a c ) ) / ( 2a )
//   x2 = ( -b - sqrt( b^2 - 4 a c ) ) / ( 2a )
//
// Stores the discriminant in D:
//
//   D = b^2 - 4 a c.
//
// Throws std::range_error if there are no real solutions.
//
// If a == 0, stores -c/b in x1 and x2.
//

struct quadratic_solver {

  quadratic_solver( double const& c , double const& b , double const& a = 1 ) ;

  double const D ;

  double const x1 ;
  double const x2 ;

} ;


//
// Return b^2 - 4 a c or throw std::range_error if the result is negative.
//

double discriminant( double const& c , double const& b , double const& a = 1 ) ;


//
// Next power of two.  Returns the smallest integer p such that p is a power of
// two and x <= p.
//

long next_power_of_two( const long x ) ;


//
// Returns the dual logarithm of \a x.
//

int log_2( long x ) ;


/// \return True if and only if \a x is a power of two.

inline bool is_power_of_two( long x )
{ return cpl::math::next_power_of_two( x ) == x ; }


/// \return The bit reversal of \a x with its lower \a l bits reversed.

long bit_reversal( long x , int l ) ;


/// Perform bit-reversal permutation on [ begin , end ).

/// \pre \a ran_it must be a random access iterator.
/// \pre \a end - \a begin must be a power of two.
/// \post Each element x[ i ] is swapped with x[ bit_reversal( i ) ].

template< typename ran_it >
void bit_reversal_permutation( ran_it const& begin , ran_it const& end ) ;


/// \return x^2.

template< typename T >
inline constexpr T square( T const& x )
{ return x * x ; }


/// A simple statically expanded power function.

/// \return x^p

template< unsigned long p , typename T >
inline T power( T const& x ) {

  if( p == 0 ) return 1 ;

  T y = square( power< p / 2 >( x ) ) ;

  return ( p % 2 ) ? y * x : y ;

}


//
// 4th order Runge-Kutta integration step for 
//
//   d/dt x = f( x , u ) 
//
// for time step dt with u constant over dt.
//
// Requires that X have addition and scalar multiplication.
//

template< typename X , typename U , typename F >
inline X const rk_update
( X const& x , U const& u , F const& f , double const& dt ) {

  X const k1 = dt * f( x           , u ) ;
  X const k2 = dt * f( x + .5 * k1 , u ) ;
  X const k3 = dt * f( x + .5 * k2 , u ) ;
  X const k4 = dt * f( x +      k3 , u ) ;

  return x 
    + ( 1 / 6. ) * k1 
    + ( 1 / 3. ) * k2 
    + ( 1 / 3. ) * k3 
    + ( 1 / 6. ) * k4 
  ;

}


//
// Model block:  Low pass.
//
// Very simple special case of a linear system, this models the equation
//
//   TC dx/dt = u - x
//
// where u is model input, x is model state (equal to output y), TC the time
// constant.  TC shouldn't be zero and can be set dynamically with each
// derivatives() call.
//
// See http://en.wikipedia.org/wiki/Lowpass_filter.
//

template< typename T = double >
struct low_pass {

  // State type, may be float or double.
  typedef T state_type ;

  T default_state() const { return T() ; }

  low_pass() : y( T() ) {}

  // Compute (set) outputs.
  void outputs( state_type const& x , T const& u ) {
    // Low-pass output doesn't depend on input.
    static_cast<void>(u);
    y = x ;
  }

  // Compute derivative with given time constant TC != 0.
  state_type derivatives( state_type const& x , T const& u , T const& TC ) const
  { assert( TC != T( 0 ) ) ; return ( u - x ) / TC ; }

  // The output.
  T y ;

} ;


//
// Type to be used as the state for the second_order block below.
//

template< typename T = double > struct second_order_state {

  second_order_state( T const& x , T const& v ) : x( x ) , v( v ) {}

  second_order_state() : x( 0 ) , v( 0 ) {}

  second_order_state const& operator*=( T const& s )
  { x *= s ; v *= s ; return *this ; }

  second_order_state const& operator+=( second_order_state const& rhs ) {
    x += rhs.x ;
    v += rhs.v ;
    return *this ;
  }

  // Position [unspecified unit].
  T x ;
  // Velocity [unspecified unit]; implied: v = dx/dt.
  T v ;

} ;


//
// Model block:  Second order system.
//
// This models the equation
//
//   d^2 x/dt^2 + b dx/dt + a ( x - u ) = 0,
//
// transformed into a first-order system with the standard substitution
//
//   dx/dt = v.
//
// The spring constant a and damping constant b can be modified on the fly.
//
// When m > 0, the delta x - u is taken modulo m and centered around zero.
//
// The oscillation frequency is i sqrt( b^2/4 - a ), the damping exponent is
// a/2.  The system is critically damped when b^2/4 = a.
//
// This can be used to model a spring/damper system, with the modulus e.g. on
// the circle if so wished.  Cf. "Damped harmonic oscillator" on the page
// below.
//
// http://en.wikipedia.org/wiki/Harmonic_oscillator
//

template< typename T = double >
struct second_order {

  typedef second_order_state< T > state_type ;

  second_order() : a( 0 ) , b( 0 ) , m( 0 ) {}

  state_type default_state() const { return state_type() ; }

  state_type derivatives( state_type const& x , T const& u ) const {
    T dx = x.x - u ;
    if( m > 0 ) {
      dx = modulo( dx , m ) ;
      if( dx > m / 2 ) { dx -= m ; }
    }
    return state_type( x.v , - b * x.v - a * dx ) ;
  }

  T a ;
  T b ;
  T m ;

} ;

//
// TODO: average and exponential_moving_average don't fit the 'discrete model
// block' concept.  Introduce a separate discrete time modelling concept.
//
// Discrete model block:  Average.
//
// State: Sum and number of added elements.
// Output: result (the average), which is NaN if no update has been done.
//

template< typename T = double >
struct average {
  average() {}

  struct discrete_state_type {
    T sum;
    double n;

    discrete_state_type() : sum(T()), n(0) {}
  };

  discrete_state_type default_discrete_state() const
  { return discrete_state_type(); }

  void update_discrete_states(discrete_state_type& x, T const& u) const {
    ++x.n;
    x.sum += u;
  }

  T outputs(discrete_state_type const& x) const {
    if (x.n < .5) {
      return std::numeric_limits<T>::quiet_NaN();
    } else {
      return x.sum / x.n;
    }
  }
};


//
// Discrete model block:  Exponential moving average.
//
// State: Current exponential moving average, NaN if no update has been called.
// Output: none (equivalent with state)
// Parameter: The mix-in factor C.  The state x is updated as
//   x <- (1-C)*x + C*u
//


template< typename T = double >
struct exponential_moving_average {

  typedef T discrete_state_type;
  T default_discrete_state() const
  { return std::numeric_limits<T>::quiet_NaN(); }

  exponential_moving_average(double const& C) : C(C) {}

  void update_discrete_states(discrete_state_type& x, T const& u) const {
    if (isnan(x)) {
      x = u;
    } else {
      x = (1 - C) * x + C * u;
    }
  }
private:
  double const C;
};


//
// Same as the above, but with modulo
//

template< typename T = double , 
          typename InvalidStateFunction = cpl::detail_::isnan_function_object>
struct modulo_exponential_moving_average {

  typedef T discrete_state_type;

  // T default_discrete_state() const
  // { return std::numeric_limits<T>::quiet_NaN(); }

  // Provide a default constructor for good measure...
  modulo_exponential_moving_average()
  : C{T(1.0)},
    M{T(1.0)}
  {}

  modulo_exponential_moving_average(T const& C, T const& M, 
      InvalidStateFunction const& invalid = InvalidStateFunction{})
  : C{C},
    M{M},
    invalid(invalid)
  {
    always_assert(0 <= C     );
    always_assert(     C <= 1);
    always_assert(M > 0);
  }

  void update_discrete_states(discrete_state_type& x, T const& u) const {
    if (invalid(x)) {
      x = u;
    } else {
      auto const delta1 = cpl::math::modulo(u - x, M);
      auto const delta  = delta1 < M/2.0 ? delta1 : delta1 - M;
      x = cpl::math::modulo(x + C * delta, M);
    }
  }
private:
  double C;
  double M;
  InvalidStateFunction invalid;
};

//
// Discrete model block:  Zero crossings.
//
// Output: The boolean variable fire, which is set to true whenever the input
// signal is exactly zero or changed sign since the last timestep (outputs()
// call).
//

template< typename T = double >
struct zero_crossing {

  zero_crossing() : fire( false ) {}

  typedef T discrete_state_type ;

  T const default_discrete_state() const { return 0 ; }

  void outputs( discrete_state_type const& x , T const& u ) {

    fire =    u == 0
           || (u > 0 && x < 0)
           || (u < 0 && x > 0)
    ;

  }

  void update_discrete_states( discrete_state_type& x , T const& u ) const
  { x = u ; }

  bool fire ;

} ;


//
// ODE solvers.
//
// The models M must support
//
//   projection            ()
//   outputs               ()
//   update_discrete_states()
//   derivatives           ()
//   (any number of outputs()/derivatives() alternating)
//
// which are called in turn in this order.
//
// M must supply type state_type supporting addition and scalar
// multiplication for the continuous states.
//
// M::default_state() must return a suitably initialized (dimensions,...)
// state_type object.
//
// M::default_discrete_state() must return a suitably initialized
// (dimensions,...) discrete_state_type object for the discrete states.
//
// Models have a step<>() function template that advance the state one time
// step.  Model input must have been set an are assumed to be constant over dt.
// The step<>() function template accepts an function f().  The expression
// f( m , t ) for the model m and time t must be valid and can be used to
// inspect the model variables at a precise point during the integration,
// namely after the first output() call.  f() is passed a const M& m, it's not
// allowed to modify variables!
//

//
// Euler solver.
//

template< typename M >
struct euler_solver : cpl::detail_::ode_solver_base< M > {

  euler_solver( M& m , double const& dt )
  : cpl::detail_::ode_solver_base< M >( m , dt ) ,
    k1( m.default_state() )
  {}

  template< typename F >
  void step( F const& f ) {

    base::m.projection( base::x ) ;

    base::m.outputs( base::x , base::xd ) ;
    base::m.update_discrete_states( base::xd ) ;

    f( base::model() , base::time() ) ;

    k1 = base::m.derivatives( base::x ) ; k1 *= base::dt ;
    base::x += k1 ;

    base::t += base::dt ;

  }

private:

  typedef cpl::detail_::ode_solver_base< M > base ;

  typename M::state_type k1 ;

} ;


//
// 4th order Runge-Kutta solver.
//

template< typename M > struct rk4_solver : cpl::detail_::ode_solver_base< M > {

  rk4_solver( M& m , double const& dt )
  : cpl::detail_::ode_solver_base< M >( m , dt ) ,
    k1 ( m.default_state() ) ,
    k2 ( m.default_state() ) ,
    k3 ( m.default_state() ) ,
    k4 ( m.default_state() ) ,
    arg( m.default_state() )
  {}

  template< typename F >
  void step( F const& f ) {

    base::m.projection( base::x ) ;

    base::m.outputs( base::x , base::xd ) ;
    base::m.update_discrete_states( base::xd ) ;

    f( base::model() , base::time() ) ;

    k1 = base::m.derivatives( base::x   ) ; k1 *= base::dt ;

    arg = k1 ; arg *= .5 ; arg += base::x ;
    base::m.outputs( arg , base::xd ) ;
    k2 = base::m.derivatives( arg ) ; k2 *= base::dt ;

    arg = k2 ; arg *= .5 ; arg += base::x ;
    base::m.outputs( arg , base::xd ) ;
    k3 = base::m.derivatives( arg ) ; k3 *= base::dt ;

    arg = k3 ;             arg += base::x ;
    base::m.outputs( arg , base::xd ) ;
    k4 = base::m.derivatives( arg ) ; k4 *= base::dt ;

    k1 *= .1666666666666666666666666666666666 ;
    k2 *= .3333333333333333333333333333333333 ;
    k3 *= .3333333333333333333333333333333333 ;
    k4 *= .1666666666666666666666666666666666 ;

    base::x += k1 ;
    base::x += k2 ;
    base::x += k3 ;
    base::x += k4 ;

    base::t += base::dt ;

  }

private:

  typedef cpl::detail_::ode_solver_base< M > base ;

  typename M::state_type k1 , k2 , k3 , k4 , arg ;

} ;


/// Numerical derivative of a vector.

/// Compute numerical derivative of according to the symmetric formula
/// \f[
/// f^{ \prime }( x ) \approx \frac{ f( x + h ) - f( x - h ) }{ 2h }.
/// \f]
/// At the beginning and end of the vector, the asymmetric formula is
/// used.
///
/// \param v Tabulated function values, 1/\a f apart.
/// \param f Sampling frequency.
///
/// \return Tabulated function values of the derivative.  Same number of
/// elements as in \a v.

template< typename T >
std::vector< T >
derivative( std::vector< T > const& v , double const& f ) ;


/// Convert \a cont to std::vector< double >

/// \return Some sequence of some element type converted to a
/// double vector.

template< typename C >
inline std::vector< double >
to_double_vector( C const& cont ) {

  return std::vector< double >( cont.begin() , cont.end() ) ;

}


/// Report locations and values of Minima and Maxima in a vector.

/// The message format is:
///
///   name: maximum max at t = t_max , minimum min at t = t_min \n
///
/// \param os Stream to write to.
/// \param v The vector.
/// \param name The beginning of the message string.
/// \param frequency Sampling frequency, used for time
/// computation.

template< typename T >
void report_minmax(
  std::ostream& os ,
  std::vector< T > const& v ,
  std::string const& name ,
  long frequency
) ;


/// Sample a function.

/// Fill a container with sampled values, starting at t0 with frequency f.
///
/// \retval cont Where to write to.
/// \param function A unary function.
/// \param t0 The starting time.
/// \param f Frequency.

template< typename C , typename F , typename T >
void sample(
  C& cont ,
  F const& function ,
  T const& t0 ,
  T const& f
) ;


/// Multiply all elements of container \a c by \a x.

template< typename C >
void multiply( C& c , typename C::value_type const& x ) ;


/// assert() that all elements in range are >= 0

/// \param begin Iterator pointing to the beginning element of the range.
/// \param end Iterator pointing to the past-the-end element of the
/// range.

template< typename for_it >
void assert_nonneg( for_it const& begin , for_it const& end ) ;


/// Do downsampling.

/// Return a vector where each element is the mean of n succesive
/// elements of cont.  The cont.size() % n last elements of cont are
/// ignored.
///
/// \param cont The original value sequence.
/// \param n Downsampling factor.
/// \return The vector of downsampled values.

template< typename C >
std::vector< typename C::value_type >
down_sample( C const& cont , long n ) {

  assert( n >= 1 ) ;

  typedef typename C::value_type T ;

  std::vector< T > ret( cont.size() / n ) ;

  for( typename std::vector< T >::size_type i = 0 ; i < ret.size() ; ++i ) {

    T x = 0 ;

    for( long j = 0 ; j < n ; ++j )
      x += cont[ n * i + j ] ;

    ret[ i ] = x / n ;

  }

  return ret ;

}

//
// An inner product with diagonal weight factors.  Computes
// x^T diag(w) y
//

template <int N> struct weighted_inner_product {
  typedef cpl::matrix::vector_fixed_t<N> vector_type;

  // TODO: Assert that w(i) > 0?
  weighted_inner_product(std::vector<double> const& weight)
    : w(weight)
  { validate(); }

  weighted_inner_product(std::vector<double>&& weight)
    : w(std::move(weight))
  { validate(); }

  // Computes w(1) * x(1) * y(1) + ... + w(N) * x(N) * y(N)
  double operator()(vector_type const& x, vector_type const& y) const;

  unsigned size() const { return N; }

  void validate() const {
    cpl::util::verify(size() == w.size(),
       "wrong dimension for weighted innner product"); 
  }

  std::vector<double> const& weights() const { return w; }

private:
  std::vector<double> w;
};

// Safe function to calculate percentages.  Returns 0.0 in case
// of divisions by zero.
inline double percentage(const double& value, const double& reference) {
  if (0.0 == reference) {
    return 0.0;
  } else {
    return 100.0 * value / reference;
  }
}

} // namespace math

} // namespace cpl


//
// Template and inline definitions.
//

//
// Compute derivative.  See ``Numerical Recipes in C'', Section 5.7
// ``Numerical Derivatives'', (5.7.7).
//

template< typename T >
std::vector< T >
cpl::math::derivative( std::vector< T > const& v , double const& f ) {

  assert( f > 0 ) ;

  std::vector< T > ret( v.size() ) ;

  if( v.size() == 0 ) return ret ;

  if( v.size() == 1 ) { ret[ 0 ] = 0 ; return ret ; }


  for( typename std::vector< T >::size_type i = 1 ; i < v.size() - 1 ; ++i )

    ret[ i ] = f * ( v[ i + 1 ] - v[ i - 1 ] ) / 2 ;


  ret[ 0 ] = f * ( v[ 1 ] - v[ 0 ] ) ;

  ret.back() = f * ( *( v.end() - 1 ) - *( v.end() - 2 ) ) ;


  return ret ;

}



template< typename T >
void cpl::math::report_minmax(
  std::ostream& os ,
  std::vector< T > const& v ,
  std::string const& name ,
  long frequency
) {

  assert( v.size() ) ;

  typename std::vector< T >::const_iterator
    max = std::max_element( v.begin() , v.end() ) ;
  typename std::vector< T >::const_iterator
    min = std::min_element( v.begin() , v.end() ) ;

  os << name
     << ": maximum " << *max
     << " at t=" << static_cast< double >( max - v.begin() ) / frequency
     << ", minimum " << *min
     << " at t=" << static_cast< double >( min - v.begin() ) / frequency
     << '\n' ;

}


template< typename C , typename F , typename T >
void cpl::math::sample(
  C& c ,
  F const& fun ,
  T const& t0 ,
  T const& f
) {

  typename C::size_type j = 0 ;

  for( typename C::iterator i = c.begin() ; i != c.end() ; ++i )
    *i = fun( t0 + j++ / f ) ;

}


template< typename C >
void cpl::math::multiply( C& c , typename C::value_type const& x ) {

  for( typename C::iterator i = c.begin() ; i != c.end() ; ++i )
  { *i *= x ; }

}


template< typename for_it >
void cpl::math::assert_nonneg( for_it const& begin , for_it const& end ) {

  for( for_it i = begin ; i != end ; ++i )
    assert( *i >= 0 ) ;

}


template< typename ran_it >
void cpl::math::bit_reversal_permutation( ran_it const& begin , ran_it const& end ) {

  const long n = end - begin ;

  assert( n > 0 ) ;
  assert( is_power_of_two( n ) ) ;

  const int t = log_2( n ) ;

  if( t < 1 ) { return ; }

  for( long i = 0 ; i < n ; ++i ) {

    const long j = bit_reversal( i , t ) ;
    assert( 0 <= j && j < n ) ;

    if( j > i ) std::swap( *( begin + i ) , *( begin + j ) ) ;

  }

}

template <int N>
double cpl::math::weighted_inner_product<N>::operator()(
    vector_type const& x, vector_type const& y) const {
  double ret = 0.0;
  for (unsigned i = 0; i < w.size(); ++i) {
    ret += w[i] * x(i) * y(i);
  }
  return ret;
}

#endif // CPP_LIB_MATH_UTIL_H
