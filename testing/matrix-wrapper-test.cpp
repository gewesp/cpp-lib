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
// Tests for the Matrix/Eigen wrapper
//
// TODO: Progressively enable the #ifdef'd out tests
//


#include <iostream>
#include <numeric>
#include <random>

#include <cmath>

#include "cpp-lib/geometry.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/units.h"

using namespace cpl::matrix;


//
// Test less-than operator for matrices.
//

void matrix_sort_test( long const m , long const n , unsigned long const N ) {

  std::vector< matrix_t > v( N ) ;

  std::minstd_rand mstd;
  std::uniform_int_distribution<long> U(0, 1000000);

  for( unsigned long i = 0 ; i < N ; ++i ) {
    v[ i ].resize( m , n ) ;
    fill( v[ i ] , static_cast< double >( U( mstd ) ) ) ;
  }

  // Do a selection sort since std::sort() isn't guaranteed to work...
  for( unsigned long i = 0 ; i < N ; ++i ) {
    for( unsigned long j = i + 1 ; j < N ; ++j ) {
      if( v[ j ] < v[ i ] )
      { v[ i ].swap( v[ j ] ) ; }
    }
  }

  // Verify order.
  for( unsigned long i = 0 ; i + 1 < N ; ++i ) {
    always_assert( v[ i ] < v[ i + 1 ] ) ;
  }

}

//
// Test transitivity of less-than operator.
//

void matrix_less_test( long const m , long const n , unsigned long const N ) {

  std::minstd_rand mstd;
  std::uniform_int_distribution<long> U(0, 1000000);

  for( unsigned long i = 0 ; i < N ; ++i ) {
    matrix_t x( m , n ) ;
    matrix_t y( m , n ) ;
    matrix_t z( m , n ) ;
    fill( x , static_cast< double >( U( mstd ) ) ) ;
    fill( y , static_cast< double >( U( mstd ) ) ) ;
    fill( z , static_cast< double >( U( mstd ) ) ) ;
    // Nonreflexivity.
    always_assert( !( x < x ) ) ;
    // Antisymmetry.
    always_assert( !( x < y && y < x ) ) ;
    // Transitivity.
    always_assert( ( x < y && y < z ) <= ( x < z ) ) ;

    // Probably also true, otherwise increase the possible number of
    // random values!
    always_assert( x < y || y < x ) ;
  }

}

void matrix_order_test() {
  std::cout << "Testing matrix lexicographic ordering...\n" ;
  matrix_less_test( 10 , 10 , 1000 ) ;
  matrix_less_test( 1  , 10 , 10000 ) ;
  matrix_less_test( 10 , 1  , 10000 ) ;
  matrix_less_test( 1 , 1 , 100  ) ;
  matrix_less_test( 3 , 3 , 200 ) ;

  matrix_sort_test( 10 , 10 , 33 ) ;
  matrix_sort_test( 1  , 10 , 100 ) ;
  matrix_sort_test( 10 , 1  , 200 ) ;
  matrix_sort_test( 1 , 1 , 1  ) ;
  matrix_sort_test( 1 , 1 , 10  ) ;
  matrix_sort_test( 3 , 3 , 200 ) ;
  std::cout << "PASS" << std::endl;
}

void geometry_test( vector_2_t const& v1 , vector_2_t const& v2 ) {
  std::cout << "v1 = " << v1 << std::endl
            << "v2 = " << v2 << std::endl ;
  std::cout << "angle(v1, v2) = " 
            << cpl::math::signed_angle(v1, v2) / cpl::units::degree()
            << std::endl;
  std::cout << "angle(v2, v1) = " 
            << cpl::math::signed_angle(v2, v1) / cpl::units::degree()
            << std::endl;
}

void geometry_test() {
  std::cout << "Testing signed_angle()...\n";
  {
    auto const v1 = column_vector( 1.0 , 0.0 ) ;
    geometry_test( v1 , column_vector( 1.0 , 0.0 ) ) ;
    geometry_test( v1 , column_vector( 0.0 , 1.0 ) ) ;
    geometry_test( v1 , column_vector( 1.0 , 1.0 ) ) ;
    geometry_test( v1 , column_vector( -1.0 , -1.0 ) ) ;
  }
  {
    auto const v1 = column_vector( 1e-30 , 1e-28 ) ;
    geometry_test( v1 , column_vector( 1.0 , 0.0 ) ) ;
    geometry_test( v1 , column_vector( 0.0 , 1.0 ) ) ;
    geometry_test( v1 , column_vector( 1.0 , 1.0 ) ) ;
  }
}

#if 0

void math_util_test() {

  always_assert(  approximate_equal( 0 , 0 ) ) ;
  always_assert(  approximate_equal( 0 , 0 , 1e-100 ) ) ;
  always_assert(  approximate_equal( 0 , 0 , 1e100  ) ) ;
  always_assert(  approximate_equal( 1 , 1 ) ) ;
  always_assert( !approximate_equal( 1 , 1.00000000001 ) ) ;
  always_assert(  approximate_equal( 1 , 1.0000000000001 , 1e8 ) ) ;

  std::cout << "Testing mathematics utilities..." << std::endl ;
  {
    // (2x + 3)(4x - 1)
    quadratic_solver qs( -3 , 10 , 8 ) ;
    std::cout << qs.x1 << std::endl ;
    std::cout << qs.x2 << std::endl ;
    always_assert( approximate_equal( qs.x1 ,   1. / 4. ) ) ;
    always_assert( approximate_equal( qs.x2 , - 3. / 2. ) ) ;
  }
  {
    // (3x - 2)(2x + 5)
    quadratic_solver qs( -10 , 11 , 6 ) ;
    std::cout << qs.x1 << std::endl ;
    std::cout << qs.x2 << std::endl ;
    always_assert( approximate_equal( qs.x1 ,   2. / 3. ) ) ;
    always_assert( approximate_equal( qs.x2 , - 5. / 2. ) ) ;
  }
  {
    // Linear: 3x - 2
    quadratic_solver qs( -2 , 3 , 0 ) ;
    std::cout << qs.x1 << std::endl ;
    std::cout << qs.x2 << std::endl ;
    always_assert( approximate_equal( qs.x1 , .66666666666666666666666666 ) ) ;
    always_assert( approximate_equal( qs.x2 , .66666666666666666666666666 ) ) ;
  }

  std::cout << "Success." << std::endl ;

}
#endif

void rotation_matrix_test(std::ostream& os) {
  auto const rot45 = 
    Eigen::Rotation2D<double>(45 * cpl::units::degree()).toRotationMatrix();

  matrix_2_t const rot90    = rot45  * rot45 ;
  matrix_2_t const rot180   = rot90  * rot90 ;
  matrix_2_t const identity = rot180 * rot180;

  // os << rot45    << std::endl;
  // os << rot90    << std::endl;
  // os << rot180   << std::endl;
  // os << identity << std::endl;

  matrix_2_t expected_rot45   ;
  matrix_2_t expected_rot90   ;
  matrix_2_t expected_rot180  ;
  matrix_2_t expected_identity;
  expected_rot45  << 0.7071067811865476 , -0.7071067811865476 ,
                     0.7071067811865476 ,  0.7071067811865476
  ;
  expected_rot90  << 0 , -1 ,
                     1 ,  0
  ;
  expected_rot180 << -1 ,  0 ,
                      0 , -1
  ;
  expected_identity << 1 , 0 ,
                       0 , 1
  ;

  always_assert((rot45    - expected_rot45   ).squaredNorm() <= 1e-14);
  always_assert((rot90    - expected_rot90   ).squaredNorm() <= 1e-14);
  always_assert((rot180   - expected_rot180  ).squaredNorm() <= 1e-14);
  always_assert((identity - expected_identity).squaredNorm() <= 1e-14);
}

int main() {
  geometry_test();
  matrix_order_test();
  rotation_matrix_test(std::cout);

  matrix_3_t O;
  O << 1 , -1 , 0 ,
       1 ,  1 , 0 ,
       0 ,  0 , 1
  ;
  std::cout << "O = \n" << O << std::endl;

  std::cout << "O^T = " << transpose( O ) << std::endl ;
  std::cout << "O * O^T = " << O * transpose( O ) << std::endl ;

  std::cout << "2 * O = " << 2.0 * O << std::endl;

  auto const x = column_vector( 2. ,  2. , 4. ) ;
  auto const y = column_vector( 0. , -4. , 2. ) ;

  std::cout << "x = " << x << std::endl;
  std::cout << "y = " << y << std::endl;
  std::cout << "x + y = " << x + y << std::endl;
  std::cout << "x|y = " << inner_product(x, y) << std::endl;
  std::cout << "x|y = " << (x|y) << std::endl;
  std::cout << "y^T = " << transpose(y) << std::endl;

  std::cout << "norm_2(x) = " << norm_2(x) << std::endl;
}
