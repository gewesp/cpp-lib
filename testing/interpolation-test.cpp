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


#include <any>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <string>

#include "cpp-lib/registry.h"
#include "cpp-lib/interpolation.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/util.h"
#include "cpp-lib/sys/file.h"

using namespace cpl::math       ;
using namespace cpl::util       ;
using namespace cpl::util::file ;


namespace {

double const dx = .2 ;


template< typename F >
void write_surface
( std::string const& filename , registry const& reg , F const& f ) {
  
  std::vector< double > const param = 
    reg.check_vector_double( "surface_range" , 5 ) ;

  double const x_min = param[ 0 ] ;
  double const x_max = param[ 1 ] ;
  double const y_min = param[ 2 ] ;
  double const y_max = param[ 3 ] ;
  double const d     = param[ 4 ] ;

  auto os = open_write( filename ) ;
  
  std::cout 
    << "writing interpolated surface to " 
    << filename 
    << "..."
    << std::flush ;

  std::vector< double > x( 2 ) ;
  for( x[ 1 ] = y_min ; x[ 1 ] <= y_max ; x[ 1 ] += d ) {

    for( x[ 0 ] = x_min ; x[ 0 ] <= x_max ; x[ 0 ] += d ) 
    { os << f( x ) << ' ' ; }

    os << '\n' ;

  }
  
  std::cout << " done." << std::endl ;

}

template< typename alg >
void check_surface( registry const& reg ) {

  interpolator< alg > const f = make_interpolator< alg >( reg , "surface" ) ;

  std::string const filename = 
    reg.template get< std::string >( "surface_filename" )
    + ( 
      typeid( alg ) == typeid( simplicial< double > ) ?
      ".simplicial" : ".hypercubic" 
    ) ;

  write_surface( filename , reg , f ) ;

}


void surface_recursive( registry const& reg ) {

  recursive_interpolation<> const 
	f( reg.get_any( "surface_recursive" ) ) ;

  std::string const filename = 
	reg.get< std::string >( "surface_filename" ) + ".recursive" ;

  write_surface( filename , reg , f ) ;

}


void surfaces( registry const& reg ) {

  check_surface< hypercubic< double > >( reg ) ;
  check_surface< simplicial< double > >( reg ) ;

  surface_recursive( reg ) ;

}


void check_recursive_interpolation( registry const& reg ) {

  cpl::math::recursive_interpolation<> f
  ( reg.get_any( "recursive_interpolation" ) ) ;

  std::vector< double > x( f.dimension() ) ;

  while( std::cin ) {

	std::cout << "enter " << x.size() << " values: " << std::flush ;

	for( unsigned long i = 0 ; i < x.size() ; ++i ) 
	{ std::cin >> x.at( i ) ; }

	if( std::cin ) 
	{ std::cout << f( x.begin() , x.end() ) << std::endl ; }

  }

}


void interactive( registry const& reg ) {

  check_recursive_interpolation( reg ) ; 

}


void check_index_mapper( registry const& reg ) {

  std::cout << "index_mapper for x = -1 : .1 : 4\n" ;
  std::vector< double > const xs = reg.check_vector_double( "xs" ) ;

  index_mapper i( xs ) ;

  for( double x = -1 ; x <= 4 ; x += .1 ) 
  { std::cout << i( x ) << "\n" ; }

  std::vector< double > xxs( 3 ) ;

  bool ok = false ;
  xxs[ 0 ] =  0 ;
  xxs[ 1 ] =  1 ;
  xxs[ 2 ] = -1 ;

  try { index_mapper ii( xxs ) ; }
  catch( std::exception const& e ) {

	ok = true ;
	std::cout << "index_mapper correctly checks for ascending sequence: "
	          << e.what()
			  << '\n' ;

  }

  if( !ok ) 
  { throw std::runtime_error
	( "index_mapper fails to recognize non-ascending sequence" ) ; }

}


void check_constant( registry const& reg ) {

  std::vector< std::vector< double > > vv = 
	reg.check_vector_vector_double( "constant" , 2 , -2 ) ;

  linear_interpolation<> c = make_linear_interpolation( vv ) ;

  always_assert( c( 0    ) == vv[ 1 ][ 0 ] ) ;
  always_assert( c( 1    ) == vv[ 1 ][ 0 ] ) ;
  always_assert( c( -1   ) == vv[ 1 ][ 0 ] ) ;
  always_assert( c( 4710 ) == vv[ 1 ][ 0 ] ) ;
  always_assert( c( 4712 ) == vv[ 1 ][ 0 ] ) ;

  std::cout << "constant interpolation: OK\n" ;

}

#if 0
void check_ndim( registry const& reg ) {
  // 2-dimensional currently unsupported due to Eigen problems,
  // see comments in interpolation.h
  auto const f = make_linear_interpolation_ndim<2>(reg, "interp_2dim");

  always_assert(norm_2(f(0   ) - column_vector(1.0, 1.0)) <= 1e-9);
  always_assert(norm_2(f(4   ) - column_vector(1.0, 1.0)) <= 1e-9);
  always_assert(norm_2(f(4.5 ) - column_vector(0.5, 0.5)) <= 1e-9);
  always_assert(norm_2(f(5   ) - column_vector(0.0, 0.0)) <= 1e-9);
  always_assert(norm_2(f(5.5 ) - column_vector(1.0, 1.0)) <= 1e-9);
  always_assert(norm_2(f(1000) - column_vector(2.0, 2.0)) <= 1e-9);

  std::cout << "interpolation R -> R^2: OK\n" ;
}
#endif


} // anonymous namespace 


int main( int const argc , char const * const * const argv ) {

  try {

  cpl::util::registry reg ;
  reg.read_from( "interpolation-test.conf" , c_comments , comma_optional ) ;

  if( 2 == argc && "interactive" == std::string( argv[ 1 ] ) ) {

	interactive( reg ) ; 
	return 0 ;

  }


  check_index_mapper( reg ) ;

  linear_interpolation<> ip =
    make_linear_interpolation(
      reg.check_vector_vector_double( "interpolation" , 2 , -2 )
    ) ;

  std::vector< std::any > i1 = reg.check_vector_any( "interpolation" , 2 ) ;

  //
  // Construct hypercubic and simplicial interpolators from std::anys
  // which contain the breakpoint table (d lists of breakpoints) and the
  // d-dimensional value table.
  //
  // The dimension of the interpolator is dynamic.
  //

  interpolator< hypercubic< double > > const i1_h = 
    make_interpolator< hypercubic< double > >( i1[ 0 ] , i1[ 1 ] ) ;
  interpolator< simplicial< double > > const i1_s = 
    make_interpolator< simplicial< double > >( i1[ 0 ] , i1[ 1 ] ) ;
  recursive_interpolation<> const i1_r
  ( reg.get_any( "interpolation_recursive" ) ) ;

  std::cout 
    << "One-dimensional interpolation using interpolation<>, hypercubic<>,\n"
    << "recursive and simplicial<>.  All four columns should be the same.\n" ;

  for( double t = -2 ; t <= 2 ; t += .1 ) { 

    // 
    // Argument for multidimensional interpolators is a std::vector<>.
    //

    std::vector< double > tt( 1 ) ;
    tt[ 0 ] = t ;

    std::cout 
	  << ip  ( t  ) << ' ' 
	  << i1_h( tt ) << ' ' 
	  << i1_s( tt ) << ' ' 
	  << i1_r( tt ) << '\n' 
	; 

  }

  check_constant( reg ) ;
  // check_ndim    ( reg ) ;


  std::vector< std::any > v2 = reg.check_vector_any( "interp_2" , 2 ) ;
  std::vector< std::any > v3 = reg.check_vector_any( "interp_3" , 2 ) ;

  interpolator< hypercubic< double > > const f2 = 
    make_interpolator< hypercubic< double > >( v2[ 0 ] , v2[ 1 ] ) ;
  interpolator< hypercubic< double > > const f3 = 
    make_interpolator< hypercubic< double > >( v3[ 0 ] , v3[ 1 ] ) ;

  std::vector< double > x( 2 ) ;
  std::cout << "interp_2 (there should be a peak in the middle):\n" ;

  for( x[ 0 ] = 0 ; x[ 0 ] < 2 ; x[ 0 ] += dx ) {

    for( x[ 1 ] = 0 ; x[ 1 ] < 2 ; x[ 1 ] += dx ) 
    { std::cout << f2( x ) << ' ' ; }

    std::cout << '\n' ;

  }


  std::cout << "interp_3 at x_3 = .5 (should be zero)\n" ;
  x.resize( 3 ) ;
  x[ 2 ] = .5 ;
  
  for( x[ 0 ] = 0 ; x[ 0 ] < 2 ; x[ 0 ] += dx ) {

    for( x[ 1 ] = 0 ; x[ 1 ] < 2 ; x[ 1 ] += dx ) 
    { std::cout << f3( x ) << ' ' ; } 

    std::cout << '\n' ;

  }
  
  
  std::cout << "interp_3 at x_3 = 1.5 (should be zero)\n" ;
  x.resize( 3 ) ;
  x[ 2 ] = 1.5 ;
  
  for( x[ 0 ] = 0 ; x[ 0 ] < 2 ; x[ 0 ] += dx ) {

    for( x[ 1 ] = 0 ; x[ 1 ] < 2 ; x[ 1 ] += dx ) 
    { std::cout << f3( x ) << ' ' ; }

    std::cout << '\n' ;

  }
  
  std::cout << "interp_2 at diagonal x_1 = x_2 = x_3\n" ;

  for( x[ 0 ] = x[ 1 ] = x[ 2 ] = 0 ; x[ 0 ] < 2 ; x[ 0 ] += dx ) {

    x[ 1 ] = x[ 2 ] = x[ 0 ] ;
    std::cout << f3( x ) << ' ' ;

  }

  std::cout << '\n' ;
  
  std::cout << "interp_2( x_1 ) at x_2 = eps , x_3 = 1\n" ;

  x[ 1 ] = 1e-3 ;
  x[ 2 ] = 1 ;

  for( x[ 0 ] = 0 ; x[ 0 ] < 2 ; x[ 0 ] += dx ) 
  { std::cout << f3( x ) << ' ' ; }
  
  std::cout << '\n' ;
  
  surfaces( reg ) ;

  std::cout 
    << "Use gnuplot command ``splot <file> matrix'' to view surfaces.\n" ;

  } catch( std::runtime_error const& e )
  { die( e.what() ) ; }

}
