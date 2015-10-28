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


#include <vector>

#include <cassert>
#include <cstdlib>

#include "boost/any.hpp"

#include "cpp-lib/registry.h"
#include "cpp-lib/container-util.h"
#include "cpp-lib/interpolation.h"

using namespace cpl::math ;

linear_interpolation<> const
cpl::math::make_linear_interpolation
( std::vector< std::vector< double > > const& v ) {

  assert( v.size() == 2 ) ;
  assert( v[ 0 ].size() == v[ 1 ].size() ) ;

  return linear_interpolation<>( v[ 0 ] , v[ 1 ] ) ;

}


cpl::math::index_mapper::index_mapper
( std::vector< double > const& xs )
: xs( xs ) {

  if( xs.empty() )
  { throw std::runtime_error( "index_mapper needs at least one element" ) ; }

  cpl::util::container::check_strictly_ascending
  ( xs.begin() , xs.end() ) ;

}


void cpl::math::index_mapper::map
( double const& t , unsigned long& ii , double& f ) const {

  assert( t == t ) ;
  
  assert( size() >= 1 ) ;
  if( t <= xs.front() ) { ii = 0          ; f = 0 ; return ; }
  if( t >  xs.back () ) { ii = size() - 1 ; f = 0 ; return ; }

  // Otherwise, xs.front() < t <= xs.back() and we must have at least 2 
  // fixes.

  assert( xs.front() < t              ) ;
  assert(              t <= xs.back() ) ;

  assert( size() >= 2 ) ;

  //
  // lower_bound returns the position j of the first element s.t.
  // *j >= t.
  //
  // Thus, with i = j - 1, we have *i < t <= *j
  //

  std::vector< double >::const_iterator const i = 
    std::lower_bound( xs.begin() , xs.end() , t ) - 1 ;

  assert( xs.begin() <= i                ) ;
  assert(               i + 1 < xs.end() ) ;

  assert( *i < t               ) ;
  assert(      t <= *( i + 1 ) ) ;

  ii = i - xs.begin() ;
  f  = ( t - *i ) / ( *( i + 1 ) - *i ) ;

  assert( 0 <= f  ) ;

  assert( ii < size() ) ;

}


void cpl::math::convert( 
  std::vector< boost::any > const& v , 
  table< double >& t 
) {

  try {

  t.clear() ;

  if( 0 == v.size() ) { throw std::runtime_error( "empty (sub)table" ); }

  if( boost::any_cast< double >( &v[ 0 ] ) ) {

    for( std::size_t i = 0 ; i < v.size() ; ++i ) {

      if( double const* const d = boost::any_cast< double const >( &v[ i ] ) ) {
        t.push_back( *d ) ;
      } else { throw std::runtime_error( "not a block or nonnumeric" ) ; }

    }

  } else {

    // v[ 0 ] should hold a std::vector< double >

    for( std::size_t i = 0 ; i < v.size() ; ++i ) {

      if( std::vector< boost::any > const* const vv =
          boost::any_cast< std::vector< boost::any > >( &v[ i ] ) ) {

        table< double > tt ;

        convert( *vv , tt ) ;

        t.push_back( tt ) ;

      } else { throw std::runtime_error( "not a block or nonnumeric" ) ; }

    }

  }


  } catch( std::exception const& e ) {

    throw std::runtime_error
    ( "bad table format: " + std::string( e.what() ) ) ;

  }

}


std::vector< std::size_t >
cpl::math::cumulated_product( std::vector< std::size_t > const& v ) {

  std::vector< std::size_t > ret( v.size() + 1 ) ;

  ret[ 0 ] = 1 ;

  for( std::size_t i = 0 ; i < v.size() ; ++i ) {

    // Check that ret[ i ] * v[ i ] <= max value
    if( v[ i ] > 0 ) { 
      assert
      ( ret[ i ] < std::numeric_limits< std::size_t >::max() / v[ i ] ) ; 
    }

    ret[ i + 1 ] = ret[ i ] * v[ i ] ;

  }

  return ret ;

}
