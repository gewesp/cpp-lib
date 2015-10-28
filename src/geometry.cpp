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

#include <complex>
#include <limits>

#include <cmath>
#include <cassert>

#include "cpp-lib/math-util.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/geometry.h"


using namespace cpl::matrix ;

vector_3_t
cpl::math::spherical_to_cartesian(
  double const& theta ,
  double const& phi
) {

  vector_3_t ret ;
  ret( 0 ) = std::cos( theta ) * std::sin( phi ) ;
  ret( 1 ) = std::sin( theta ) * std::sin( phi ) ;
  ret( 2 ) =                     std::cos( phi ) ;

  return ret ;

}


void cpl::math::cartesian_to_spherical(
  vector_3_t const& v ,
  double& r           ,
  double& theta       ,
  double& phi
) {

  double const x2 = square( v( 0 ) ) ;
  double const y2 = square( v( 1 ) ) ;
  double const z2 = square( v( 2 ) ) ;

  double const& eps = std::numeric_limits< double >::epsilon() ;

  r = std::sqrt( x2 + y2 + z2 ) ;

  if( r < eps )
  { theta = phi = 0 ; return ; }

  phi = std::acos( v( 2 ) / r ) ;
  assert( phi == phi ) ;

  if( std::fabs( v( 0 ) ) < eps && std::fabs( v( 1 ) ) < eps )
  { theta = 0 ; return ; }

  theta = std::atan2( v( 1 ) , v( 0 ) ) ;
  assert( theta == theta ) ;

}


matrix_3_t
cpl::math::sphere_surface_frame
( vector_3_t const& p ) {


  double const p1s = square( p( 0 ) ) ;
  double const p2s = square( p( 1 ) ) ;
  double const p3s = square( p( 2 ) ) ;

  double const n2s = p1s + p2s ;

  double const n2 = std::sqrt( n2s ) ;

  if( n2 < std::numeric_limits< double >::epsilon() )
  { return cpl::matrix::identity<3>(); }

  double const n3s = n2s + p3s ;
  double const n1s = n2s * n3s ;

  double const n1 = std::sqrt( n1s ) ;
  double const n3 = std::sqrt( n3s ) ;

  matrix_3_t C ;

  C( 0 , 0 ) = -p( 0 ) * p( 2 ) / n1 ;
  C( 1 , 0 ) = -p( 1 ) * p( 2 ) / n1 ;
  C( 2 , 0 ) = n2s              / n1 ;
  
  C( 0 , 1 ) = -p( 1 ) / n2 ;
  C( 1 , 1 ) =  p( 0 ) / n2 ;
  C( 2 , 1 ) =  0           ;

  C( 0 , 2 ) = -p( 0 ) / n3 ;
  C( 1 , 2 ) = -p( 1 ) / n3 ;
  C( 2 , 2 ) = -p( 2 ) / n3 ;

  return C ;
 
}


double cpl::math::signed_angle( vector_2_t const& v1 , vector_2_t const& v2 ) {

  double const eps2 = cpl::math::square( 
      std::numeric_limits< double >::epsilon() ) ;

  if( cpl::matrix::inner_product( v1 , v1 ) < eps2 || cpl::matrix::inner_product( v2 , v2 ) < eps2 ) {
    return 0.0 ;
  }

  std::complex< double > z1( v1( 0 ) , v1( 1 ) ) ;
  std::complex< double > z2( v2( 0 ) , v2( 1 ) ) ;

  auto const z = z2 / z1 ;

  if( cpl::math::square( z.real() ) + cpl::math::square( z.imag() )
      < eps2 ) {
    return 0.0 ;
  }

  return std::arg( z ) ;

}


vector_2_t cpl::math::arc(double k, double s) {
  const double angle = k*s;
  return s * cpl::matrix::column_vector( sinc(angle), -cosc(angle) ) ;
}
