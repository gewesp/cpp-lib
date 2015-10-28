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


#ifndef CPP_LIB_QUATERNION_H
#define CPP_LIB_QUATERNION_H

#include <cmath>

#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/math-util.h"

namespace cpl {

namespace math {


//
// A quaternion type.
//
// Mostly implemented by looking at Hubert Holin's boost implementation
// and in [1].
//
// [1] Brian L. Stevens and Frank L. Lewis.  Aircraft control and
//   Simulation.  Second edition, Wiley, 2003.
//
// [2] Peter H. Zipfel.  Modeling and simulation of aerospace vehicle
// dynamics.  AIAA Education Series, 2000.
//

template< typename T = double > struct quaternion {

  /// Set all components to zero.

  quaternion() { q[ 0 ] = T() ; q[ 1 ] = T() ; q[ 2 ] = T() ; q[ 3 ] = T() ; }

  /// Quaternion from its four components.
  
  quaternion( T const& q0 , T const& q1 , T const& q2 , T const& q3 )
  { q[ 0 ] = q0 ; q[ 1 ] = q1 ; q[ 2 ] = q2 ; q[ 3 ] = q3 ; }

  T const& operator()( long i ) const
  { assert( 0 <= i ) ; assert( i < 4 ) ; return q[ i ] ; }

  T& operator()( long i ) 
  { assert( 0 <= i ) ; assert( i < 4 ) ; return q[ i ] ; }

private:

  T q[ 4 ] ;

} ;


template< typename T >
bool operator==( quaternion< T > const& q , quaternion< T > const& r ) {

  return 
	   q( 0 ) == r( 0 )
	&& q( 1 ) == r( 1 )
	&& q( 2 ) == r( 2 )
	&& q( 3 ) == r( 3 ) ;

}

template< typename T >
bool operator!=( quaternion< T > const& q , quaternion< T > const& r )
{ return !( q == r ) ; }


///
/// \retval The Cayley norm of \a q.
///

template< typename T >
T const norm( quaternion< T > const& q ) {

  return 
      q( 0 ) * q( 0 )
    + q( 1 ) * q( 1 )
    + q( 2 ) * q( 2 )
    + q( 3 ) * q( 3 ) ;

}


///
/// \retval The magnitude (Euclidean norm) of \a q.
///

template< typename T >
T const abs( quaternion< T > const& q ) 
{ return std::sqrt( norm( q ) ) ; }


template< typename T >
void normalize( quaternion< T >& q ) 
{ q *= T( 1 ) / abs( q ) ; }


template< typename T >
quaternion< T > const operator*( 
  quaternion< T > const& p ,
  quaternion< T > const& q
) {
  
  return quaternion< T >(
    p( 0 ) * q( 0 ) - p( 1 ) * q( 1 ) - p( 2 ) * q( 2 ) - p( 3 ) * q( 3 ) ,
    p( 1 ) * q( 0 ) + p( 0 ) * q( 1 ) - p( 3 ) * q( 2 ) + p( 2 ) * q( 3 ) ,
    p( 2 ) * q( 0 ) + p( 3 ) * q( 1 ) + p( 0 ) * q( 2 ) - p( 1 ) * q( 3 ) ,
    p( 3 ) * q( 0 ) - p( 2 ) * q( 1 ) + p( 1 ) * q( 2 ) + p( 0 ) * q( 3 )
  ) ;

}


template< typename T >
quaternion< T > const& operator*=( quaternion< T >& q , T const& s ) {

  q( 0 ) *= s ;
  q( 1 ) *= s ;
  q( 2 ) *= s ;
  q( 3 ) *= s ;

  return q ;

}


template< typename T >
inline quaternion< T > const operator*( T const& s , quaternion< T > q )
{ return q *= s ; }


template< typename T >
quaternion< T > const operator-
( quaternion< T > const& q1 , quaternion< T > const& q2 ) { 
  
  return quaternion< T >(
    q1( 0 ) - q2( 0 ) , 
    q1( 1 ) - q2( 1 ) , 
    q1( 2 ) - q2( 2 ) , 
    q1( 3 ) - q2( 3 ) 
  ) ;

}

template< typename T >
quaternion< T > const operator+=
( quaternion< T >& q1 , quaternion< T > const& q2 ) { 

  q1( 0 ) += q2( 0 ) ;
  q1( 1 ) += q2( 1 ) ;
  q1( 2 ) += q2( 2 ) ;
  q1( 3 ) += q2( 3 ) ;

  return q1 ;

}

template< typename T >
inline quaternion< T > const operator+
( quaternion< T > q1 , quaternion< T > const& q2 ) 
{ return q1 += q2 ; }


template< typename T >
quaternion< T > const conjugate( quaternion< T > const& q )
{ return quaternion< T >( q( 0 ) , -q( 1 ) , -q( 2 ) , -q( 3 ) ) ; }


template< typename T >
quaternion< T > const inverse( quaternion< T > const& q )
{ return 1 / norm( q ) * conjugate( q ) ; }


//
// Euler angles.  Of course, T should be some numeric type like double,
// float or long double.
//
// psi, theta and phi should be in radians and determine right-handed
// rotations about the respective axes.
//

template< typename T = double >
struct euler_angles {

  euler_angles() : psi( T() ) , theta( T() ) , phi( T() ) {}

  euler_angles( T const& psi , T const& theta , T const& phi ) 
  : psi( psi ) , theta( theta ) , phi( phi ) {}

  T psi   ; // Yaw
  T theta ; // Pitch
  T phi   ; // Roll

} ;



//
// Return the quaternion representing the yaw-pitch-roll sequence
// defined by \a psi, \a theta and \a phi.
// Cf. [1], 1.3.-33
//
// If q is the return value of this function, then the transformation 
//
//   rotation( q , u ) = C u
//
// where C is the yaw-pitch-roll sequence defined by the inputs.
//

template< typename T >
quaternion< T > const make_quaternion( euler_angles< T > const& ea ) {

  T const cpsi2   = std::cos( ea.psi   / 2 ) ;
  T const spsi2   = std::sin( ea.psi   / 2 ) ;
  T const ctheta2 = std::cos( ea.theta / 2 ) ;
  T const stheta2 = std::sin( ea.theta / 2 ) ;
  T const cphi2   = std::cos( ea.phi   / 2 ) ;
  T const sphi2   = std::sin( ea.phi   / 2 ) ;

  return quaternion< T >(
    cphi2 * ctheta2 * cpsi2 + sphi2 * stheta2 * spsi2 ,
    sphi2 * ctheta2 * cpsi2 - cphi2 * stheta2 * spsi2 ,
    cphi2 * stheta2 * cpsi2 + sphi2 * ctheta2 * spsi2 ,
    cphi2 * ctheta2 * spsi2 - sphi2 * stheta2 * cpsi2
  ) ;

}


//
// Convert quaternion to Euler angles.  Cf. [2], Eq. 4.82.
//

template< typename T >
euler_angles< T > const make_euler_angles( quaternion< T > const& q ) {

  T const den1 = 
      q( 0 ) * q( 0 ) 
    + q( 1 ) * q( 1 )
    - q( 2 ) * q( 2 )
    - q( 3 ) * q( 3 ) ;
  
  T const den2 = 
      q( 0 ) * q( 0 ) 
    - q( 1 ) * q( 1 )
    - q( 2 ) * q( 2 )
    + q( 3 ) * q( 3 ) ;

  T const psi = 
    std::atan2( 2. * ( q( 1 ) * q( 2 ) + q( 0 ) * q( 3 ) ) , den1 ) ;
  T const phi = 
    std::atan2( 2. * ( q( 2 ) * q( 3 ) + q( 0 ) * q( 1 ) ) , den2 ) ;

  T const arg = -2. * ( q( 1 ) * q( 3 ) - q( 0 ) * q( 2 ) ) ;

  T const theta = 
    arg <= -1 ? -pi / 2 : ( arg >= 1 ? pi / 2 : std::asin( arg ) ) ;

  return euler_angles< T >( psi , theta , phi ) ;

}


// 
// Change quaternion q so that only its yaw angle psi is affected.
//

template< typename T >
void change_psi( quaternion< T >& q , double const& psi ) { 

  // Convert to Euler angles and then back to quaternion.
  euler_angles< T > ea = make_euler_angles( q ) ;
  ea.psi = psi ;
  q = make_quaternion( ea ) ;

}


//
// Return the ``direction cosine matrix'', an orthogonal transformation 
// C so that
//
//   C v = rotation( q , v ) .
//
// Cf. [1], (1.3-32).
//

cpl::matrix::matrix_3_t make_dcm( quaternion< double > const& q ) {

  double const& q0 = q( 0 ) ;
  double const& q1 = q( 1 ) ;
  double const& q2 = q( 2 ) ;
  double const& q3 = q( 3 ) ;

  double const q0s = square( q0 ) ;
  double const q1s = square( q1 ) ;
  double const q2s = square( q2 ) ;
  double const q3s = square( q3 ) ;
  
  cpl::matrix::matrix_3_t C;

  C( 0 , 0 ) = q0s + q1s - q2s - q3s ;
  C( 1 , 1 ) = q0s - q1s + q2s - q3s ;
  C( 2 , 2 ) = q0s - q1s - q2s + q3s ;

  C( 1 , 0 ) = 2 * ( q1 * q2 - q0 * q3 ) ;
  C( 0 , 1 ) = 2 * ( q1 * q2 + q0 * q3 ) ;

  C( 2 , 0 ) = 2 * ( q1 * q3 + q0 * q2 ) ;
  C( 0 , 2 ) = 2 * ( q1 * q3 - q0 * q2 ) ;

  C( 2 , 1 ) = 2 * ( q2 * q3 - q0 * q1 ) ;
  C( 1 , 2 ) = 2 * ( q2 * q3 + q0 * q1 ) ;

  return C ;

}


//
// DCM directly from Euler angles.
//

cpl::matrix::matrix_3_t make_dcm( euler_angles< double > const& ea ) 
{ return make_dcm( make_quaternion( ea ) ) ; }


//
// Convert direction cosine matrix to Euler angles.  Cf. [1], (1.5-21).
//

inline euler_angles< double >
make_euler_angles( cpl::matrix::matrix_3_t const& C ) {

  double C13 = C( 0 , 2 ) ;
  cpl::math::clamp( C13 , -1.0 , 1.0 ) ;

  euler_angles< double > ea( 
     std::atan2( C( 0 , 1 ) , C( 0 , 0 ) ) ,
    -std::asin ( C13                     ) ,
     std::atan2( C( 1 , 2 ) , C( 2 , 2 ) )
  ) ;

  if( ea.phi != ea.phi ) { ea.phi = 0 ; }
  if( ea.psi != ea.psi ) { ea.psi = 0 ; }
  assert( ea.theta == ea.theta ) ;

  return ea ;

}

//
// Return the quaternion q associated with the direction cosine matrix
// C, so that
//
//   C v = rotation( q , v ) .
//
// Cf. [1], (1.3-34a).
//

inline quaternion< double > quaternion_from_dcm( 
    cpl::matrix::matrix_3_t const& C ) {

#if 0
  double const q0s = .25 * ( 1 + C( 1 , 1 ) + C( 2 , 2 ) + C( 3 , 3 ) ) ;
  double const q1s = .25 * ( 1 + C( 1 , 1 ) - C( 2 , 2 ) - C( 3 , 3 ) ) ;
  double const q2s = .25 * ( 1 - C( 1 , 1 ) + C( 2 , 2 ) - C( 3 , 3 ) ) ;
  double const q3s = .25 * ( 1 - C( 1 , 1 ) - C( 2 , 2 ) + C( 3 , 3 ) ) ;
#endif

  // Lazy...  For now, go via euler angles.

  auto const ea = make_euler_angles( C ) ;
  return make_quaternion( ea ) ;

}


// 
// Change direction cosine matrix so that only its yaw angle psi is affected.
//

inline void 
change_psi( cpl::matrix::matrix_3_t& C , double const& psi ) { 

  // Convert to Euler angles and then back to quaternion.
  euler_angles< double > ea = make_euler_angles( C ) ;
  ea.psi = psi ;
  C = make_dcm( make_quaternion( ea ) ) ;

}


//
// Return the quaternion ( 0 , v1 , v2 , v3 ).
//

quaternion< double > 
make_quaternion( cpl::matrix::vector_3_t const& v ) {

  return quaternion< double >( 0.0 , v( 0 ) , v( 1 ) , v( 2 ) ) ;

}

//
// Assuming that q( 0 ) is < epsilon, return the remaining 3 components
// as a 3-dimensional vector.
//

inline cpl::matrix::vector_3_t
to_vector( quaternion< double > const& q ) {

  return cpl::matrix::column_vector( q( 1 ) , q( 2 ) , q( 3 ) ) ;

}

//
// Quaternion representing a positive rotation of \a theta degrees around 
// axis \a u, which must have norm 1.
//
// Cf. [1], 1.2-20a and 1.2-20b.
//

inline quaternion< double > rotation_quaternion( 
  double const& theta ,
  cpl::matrix::vector_3_t const& u
) {

  double const st2 = std::sin( theta / 2 ) ;

  return quaternion< double >( 
    std::cos( theta / 2 ) , 
    st2 * u( 0 ) ,
    st2 * u( 1 ) ,
    st2 * u( 2 )
  ) ;

}


//
// Rotate \a v by \a q.
//
// The vector returned is u' for 
//
//   u = q^-1 * v' * q.
// 
// v' is the quaternion ( 0 , v ) for a 3-dimensional vector v.
//
// \a q must have norm 1.
//
// Cf. [1], (1.2-20b).
//

cpl::matrix::vector_3_t
inline rotation(
    quaternion< double > const& q , 
    cpl::matrix::vector_3_t const& v ) {

  return to_vector
    ( conjugate( q ) * make_quaternion( v ) * q ) ;

}


//
// Rotation so that the following associative law holds:
//
//   rotation( rotation( p , q ) , v ) == rotation( p , rotation( q , v ) )
//
// for quaternions p and q and a 3-dimensional vector v.
//

quaternion< double > 
inline rotation( quaternion< double > const& p , quaternion< double > const& q )
{ return q * p ; }


//
// Rotate v by the yaw-pitch-roll sequence given by ea.
//

inline cpl::matrix::vector_3_t rotation( 
    euler_angles< double > const& ea , 
    cpl::matrix::vector_3_t const& v ) 
{ return rotation( make_quaternion( ea ) , v ) ; }


/// Rotate \a u by \a theta radians around \a v (right-handed).  \a v
/// must have norm 1.

inline cpl::matrix::vector_3_t rotation( 
  double const& theta , 
  cpl::matrix::vector_3_t const& v ,
  cpl::matrix::vector_3_t const& u ) {

  auto const q = rotation_quaternion( theta , v ) ;
  return rotation( q , u ) ;

}


//
// A structure representing orientation.  It keeps a quaternion together
// with the associated direction cosine matrix which is computed on
// demand only.
//

template< typename T = double >
struct orientation {

  orientation() : q_( 1 , 0 , 0 , 0 ) , valid( false ) {}
  orientation( quaternion< T > const& q ) : q_( q ) , valid( false ) {}

  orientation< T > const& operator=( quaternion< T > const& q ) { 
	q_ = q ;
	valid = false ;
  }
  
  quaternion< T > const& q() const { return q_ ; }

  cpl::matrix::matrix_3_t const& C() const { 
	
	recompute() ;
	return C_ ; 
  
  }

private:

  void recompute() { 

	if( valid ) { return ; }
	C_ = make_dcm( q_ ) ;
	valid = true ;

  }

  quaternion< T > q_ ;
  cpl::matrix::matrix_3_t C_ ;
  mutable bool valid ;

} ;


} // namespace math

} // namespace cpl
    
#endif // CPP_LIB_QUATERNION_H
