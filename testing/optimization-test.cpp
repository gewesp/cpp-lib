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
// TODO:
// - Test minimize() with sums of Gaussians and other simple examples.
// - Test minimize() with negative length arguments.
//

#include <iostream>
#include <iomanip>
#include <random>
#include <vector>

#include "cpp-lib/optimization.h"
#include "cpp-lib/minimize.h"
#include "cpp-lib/math-util.h"
#include "cpp-lib/matrix-wrapper.h"

using namespace cpl::math ;


namespace {

void output_delta(
    std::ostream& os ,
    const double delta ,
    const double threshold ) {
  always_assert(delta <= threshold or delta >= 1.5);
  os << "delta";
  if ( delta <= threshold ) {
    os << " < " << threshold ;
  } else {
    os << " >= 1.5" ;
  }
  os << std::endl ;
}

//
// Simple 1-dimensional quadratic (x-a)^2.
//

struct quadratic {

  quadratic( double const& a ) : a( a ) {}

  void evaluate( cpl::matrix::vector_t const& x , 
                 double& fx , 
                 cpl::matrix::vector_t& dfx ) const {

    always_assert( 1 == x.rows() ) ;
    fx = square( x( 0 ) - a ) ;
    dfx.resize( 1 ) ;
    dfx( 0 ) = 2 * ( x( 0 ) - a ) ;

  }

  double const a ;

} ;


void minimize_test_quadratic( double const a , double const x0 ) {

  long const maxit = 100 ;

  quadratic q( a ) ;

  long it = 0 ;
  cpl::matrix::vector_t const x_min =
      minimize( cpl::matrix::column_vector( x0 ) , q , maxit , 1. , &it ) ;

  always_assert( 1 == x_min.rows() ) ;
  if( relative_error( x_min( 0 ) , a ) >= 1e-12 ) {
    std::cout << std::setprecision( 14 )
              << "*** FAILED ***\n"
              << "x_min found: "
              << x_min( 0 )
              << "; expected: "
              << a
              << std::endl
    ;
    always_assert( false ) ;
  }
  double fx ;
  cpl::matrix::vector_t dfx ;
  q.evaluate( x_min , fx , dfx ) ;
  always_assert( std::fabs( fx ) <= 1e-10 ) ;
  always_assert( cpl::matrix::norm_2( dfx ) <= 1e-10 ) ;
  always_assert( it <= maxit ) ;

}


void minimize_test_quadratic() {

  std::cout << "Testing minimization of (x-a)^2..." << std::endl ;

  minimize_test_quadratic( 2.5   ,  0    ) ;
  minimize_test_quadratic( .1    ,  123  ) ;
  minimize_test_quadratic( 100   , -123  ) ;
  minimize_test_quadratic( -4711 ,  4711 ) ;

  std::cout << "PASS" << std::endl;

}


// h for numerical derivatives.
const double h = 1e-6 ;

// A trivial quadratic: (x - a)^2
struct f1 {

  f1( cpl::matrix::vector_t const& a ) : a( a ) {}

  double operator()( cpl::matrix::vector_t const& x ) const {
    auto const d = x - a;
    return cpl::matrix::inner_product( d , d ) ;
  }

  cpl::matrix::vector_t a ;

} ;


//
// Test computation of numerical gradient function on (x - a)^2 for several
// dimensions and for various values of a.
//
// grad ( x - a )^2 = 2 * (x - a)
//


void gradient_test() {

  // Sigma for normal approximation.
  const double sigma = 3 ;
  std::cout << "Testing computation of numerical gradients in d dimensions."
            << std::endl ;
  std::minstd_rand mstd;
  std::normal_distribution<double> N(0.0, sigma);

  for( unsigned d = 1 ; d <= 5 ; ++d ) {

    std::cout << "d = " << d << std::endl ;

    for( int i = 0 ; i < 10 ; ++i ) {

      cpl::matrix::vector_t a( d ) ;
      cpl::matrix::vector_t x( d ) ;

      for( unsigned j = 0 ; j < d ; ++j ) {
        a( j ) = N( mstd ) ;
        x( j ) = N( mstd ) ;
      }

      f1 const f( a ) ;
      cpl::matrix::vector_t const df1 = numerical_gradient( f , x , h ) ;
      cpl::matrix::vector_t const df2 = 2. * ( x - a ) ;
      const double relerr =   cpl::matrix::norm_2( df1 - df2 ) 
                            / cpl::matrix::norm_2( df2 ) ;
      always_assert( relerr < 1e-9 ) ;
      // std::cout << "relerr = " << relerr << std::endl ;

    }

  } // for( dimension )

}

void rosenbrock_gradient_test() {

  // Sigma for normal approximation.  Rosenbrock is order |x|^4, so keep it
  // low.
  const double sigma = 3 ;
  std::cout << "Testing exact vs. numerical computation of the gradient of "
            << "the Rosenbrock function in d dimensions." << std::endl ;
  std::minstd_rand mstd;
  std::normal_distribution<double> N(0.0, sigma);

  for( unsigned d = 2 ; d <= 8 ; ++d ) {

    std::cout << "d = " << d << std::endl ;

    for( int i = 0 ; i < 6 ; ++i ) {

      cpl::matrix::vector_t x( d ) ;

      for( unsigned j = 0 ; j < d ; ++j ) {
        x( j ) = N( mstd ) ;
      }

      // std::cout << "x = " << transpose( x ) ;
      // std::cout << "r(x) = " << rosenbrock( x ) << std::endl ;
      cpl::matrix::vector_t const df1 = rosenbrock_gradient( x ) ;
      cpl::matrix::vector_t const df2 =
          numerical_gradient( &rosenbrock , x , h ) ;
      const double relerr =   cpl::matrix::norm_2( df1 - df2 ) 
                            / cpl::matrix::norm_2( df2 ) ;
      // std::cout << "d gradient = " << transpose( df1 - df2 ) ;
      always_assert( relerr <= 1e-9 ) ;
      // std::cout << "relerr = " << relerr << std::endl ;

    }

  } // for( dimension )

}

//
// Check that number of line searches (n_ls) matches what has been
// experimentally determined from the matlab implementation.
//

void minimize_test_rosenbrock_2dim(
  double const x0 ,
  double const y0 ,
  int const n_ls_max
) {

  long const maxit = 100 ;

  cpl::matrix::vector_t const x_start = cpl::matrix::column_vector( x0 , y0 ) ;
  cpl::matrix::vector_t x_min ;
  std::vector< double > fx ;
  long n_fe = 0 ;
  long n_ls = 0 ;
  minimize( x_start , rosenbrock_f() , maxit , 1 , &n_ls , &n_fe , &fx ) ;
  // Leave some slack for numerical inaccuracies here...
  always_assert( std::abs( n_ls_max
                           - static_cast< long >( fx.size() ) ) <= 2 ) ;
  always_assert( n_ls == static_cast< long >( fx.size() + 1 ) ) ;

}

void minimize_test_rosenbrock_compare_matlab() {

  std::cout << "Minimizing 2-dimensional Rosenbrock function for specific "
            << "starting values.\n" ;

  minimize_test_rosenbrock_2dim(  6.91 ,   8.77 , 32 ) ;
  minimize_test_rosenbrock_2dim( -2.32 ,   1.77 , 26 ) ;
  minimize_test_rosenbrock_2dim( -1.23 ,  -4.56 , 23 ) ;
  minimize_test_rosenbrock_2dim( 10    , 100    , 56 ) ;
  minimize_test_rosenbrock_2dim( 30    ,  10    , 61 ) ;
  minimize_test_rosenbrock_2dim(  1    ,   1    , 1  ) ;

  std::cout << "PASS" << std::endl;

}

void minimize_test_rosenbrock() {

  std::cout << "Minimizing d-dimensional Rosenbrock function for random "
            << "starting values.\n" ;
  std::cout << "Calculating distance to actual minimum " << "at (1,...,1).\n" ;
  std::cout << "For d >= 4, this may not be zero, because the function has "
            << "a second local minimum. "
            << "Only if the gradient at the minimum is too large, the test "
            << "fails."
            << std::endl
  ;

  double const sigma_0 = 4 ;
  long const maxit = 500 ;

  std::minstd_rand mstd;
  std::normal_distribution<double> N(0.0, sigma_0);

  for( int d = 2 ; d <= 10 ; ++d ) {

    std::cout << "d = " << d << std::endl ;
    // Expected location of minimum: (1,...,1).
    cpl::matrix::vector_t x_ex( d ) ;
    cpl::matrix::fill( x_ex, 1.0 ) ;

    for( int i = 0 ; i < 100 ; ++i ) {

      cpl::matrix::vector_t x0( d ) ;
      for( int j = 0 ; j < d ; ++j ) {
        x0( j ) = 1 + N( mstd ) / d;
      }

      std::vector< double > fx ;
      long n_ls = 0 ;
      cpl::matrix::vector_t const x_min =
          minimize( x0 , rosenbrock_f() , maxit , 1. , &n_ls ) ;

      always_assert( x_min.rows() == d ) ;

      double const err = cpl::matrix::norm_2( x_min - x_ex ) ;
      double const n_grad = cpl::matrix::norm_2( rosenbrock_gradient( x_min ) ) ;
      if( n_grad > 3e-5 ) {
        std::cout << "*** FAILED ***\n"
                  << "x0 = "
                  << cpl::matrix::transpose( x0 )
                  << "line searches: "
                  << n_ls
                  << "\nx_min = " << cpl::matrix::transpose( x_min )
                  << "grad f(x_min) = "
                  << cpl::matrix::transpose( rosenbrock_gradient( x_min ) )
        ;
        always_assert( false ) ;
      }
      ::output_delta( std::cout , err , 1e-6 ) ;

    }

  }

  std::cout << "PASS" << std::endl;

}

struct quadratic_n {

  quadratic_n( cpl::matrix::vector_t const& a ) : a( a ) {}

  double evaluate( cpl::matrix::vector_t const& x ) const {
    cpl::matrix::vector_t const d = x - a ;
    return d|d;
  }

  double distance( cpl::matrix::vector_t const& x , 
                   cpl::matrix::vector_t const& y ) const {
    return cpl::matrix::norm_2( x - y ) ;
  }

  cpl::matrix::vector_t a ;

} ;


// Rosenbrock adapter for downhill simplex optimizer.

struct rosenbrock_ds {

  double evaluate( cpl::matrix::vector_t const& x ) const {
    return cpl::math::rosenbrock( x ) ;
  }

  double distance( cpl::matrix::vector_t const& x , 
                   cpl::matrix::vector_t const& y ) const {
    return cpl::matrix::norm_2( x - y ) ;
  }

} ;

void ds_test_rosenbrock() {

  std::cout << "Testing Nelder-Mead downhill simplex algorithm for\n" ;
  std::cout << "Rosenbrock's banana function.\n" ;

  std::minstd_rand mstd;
  std::normal_distribution<double> N(0.0, 10.0);

  for( int d = 2 ; d <= 8 ; ++d ) {
    std::cout << "Downhill simplex for " << d << "-dimensional Rosenbrock.\n" ;
    cpl::matrix::vector_t x_expected( d ) ;
    cpl::matrix::fill( x_expected , 1.0 ) ;
    std::vector< cpl::matrix::vector_t > x0( d + 1 ) ;

    for( int repeat = 0 ; repeat < 20 ; ++repeat ) {

      cpl::matrix::vector_t a( d ) ;
      for( int i = 0 ; i < d + 1 ; ++i ) {
        x0[ i ].resize( d ) ;
        for( int j = 0 ; j < d ; ++j ) {
          x0[ i ]( j ) = N( mstd );
        }
      }

      // Nice example of C++'s Function/Object ambiguity.  Cf.
      // Gotcha #19 in Steve Dewhurst's C++ Gotchas (Addison-Wesley).
      // const rosenbrock_ds r ;  // g++ says:  uninitialized const r
      // const rosenbrock_ds r() ;  // Interpreted as a function declaration.
      const rosenbrock_ds r = rosenbrock_ds() ;  // workaround.

      // Turns out that for the Rosenbrock function beta=0.95 works much better
      // than 0.5.  Up to d=8, results are not so bad.
      auto const x = 
        downhill_simplex( x0 , r , 10e-12 , 10e-8 , 1000000 , 1 , .95 ) ;

      ::output_delta( std::cout, cpl::matrix::norm_2( x - x_expected ) , 1e-9 ) ;

    }

  }

}

void ds_test_quadratic() {

  std::cout << "Testing Nelder-Mead downhill simplex algorithm for\n" ;
  std::cout << "a quadratic function.\n" ;

  std::minstd_rand mstd;
  std::normal_distribution<double> N(0.0, 10.0);

  for( int d = 1 ; d <= 7 ; ++d ) {

    std::cout << "Downhill simplex for " << d << "-dimensional quadratic.\n" ;
    std::vector< cpl::matrix::vector_t > x0( d + 1 ) ;
    for( int repeat = 0 ; repeat < 30 ; ++repeat ) {
      cpl::matrix::vector_t a( d ) ;
      for( int i = 0 ; i < d + 1 ; ++i ) {
        x0[ i ].resize( d ) ;
        for( int j = 0 ; j < d ; ++j ) {
          x0[ i ]( j ) = N( mstd ) ;
        }
      }
      for( int j = 0 ; j < d ; ++j ) {
        a( j ) = 3.0 * N( mstd ) ;
      }

      const quadratic_n q( a ) ;
      auto const x = 
          downhill_simplex( x0 , q , 1e-12 , 1e-12 , 10000000 ,
                                           1 , .7 ) ;

      ::output_delta( std::cout, cpl::matrix::norm_2( x - a ) , 1e-9 ) ;

    }

  }

}

} // anonymous namespace

int main() {

  ds_test_rosenbrock() ;
  ds_test_quadratic () ;

             gradient_test() ;
  rosenbrock_gradient_test() ;

  minimize_test_quadratic                () ;
  minimize_test_rosenbrock_compare_matlab() ;
  minimize_test_rosenbrock               () ;

}
