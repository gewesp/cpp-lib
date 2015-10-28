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


#ifndef CPP_LIB_ARRAY_OPS_H
#define CPP_LIB_ARRAY_OPS_H

#include "boost/array.hpp"

#include <functional>

#include <cstdlib>
#include <cmath>


/// \return \a a + \a b.

template < class T , std::size_t n >
inline boost::array< T , n > operator+
( boost::array< T , n > a , boost::array< T , n > b ) {

  boost::array< T , n > ret ;

  for( typename boost::array< T , n >::size_type i = 0 ; i < n ; ++i )

    ret[ i ] = a[ i ] + b[ i ] ;

  return ret ;

}


/// \return \a a - \a b.

template < class T , std::size_t n >
inline boost::array< T , n > operator-
( boost::array< T , n > a , boost::array< T , n > b ) {

  boost::array< T , n > ret ;

  for( typename boost::array< T , n >::size_type i = 0 ; i < n ; ++i )

    ret[ i ] = a[ i ] - b[ i ] ;

  return ret ;

}


/// \return \a l * \a a.

template < class T , std::size_t n >
inline boost::array< T , n > operator*
( T l , boost::array< T , n > a ) 
{

  boost::array< T , n > ret ;

  for( typename boost::array< T , n >::size_type i = 0 ; i < n ; ++i )

    ret[ i ] = l * a[ i ] ;

  return ret ;

}


/// Add \a b to \a a and return \a a.

template< typename T , std::size_t n >
inline boost::array< T , n >& operator+=
( boost::array< T , n >& a , boost::array< T , n > const& b )
{
  for( std::size_t i = 0 ; i < n ; ++i )
    a[ i ] += b[ i ] ;

  return a ;
}


/// Divide each element of \a a by \a l and return \a a.
template< typename T , std::size_t n >
inline boost::array< T , n >& operator/=
( boost::array< T , n >& a , T l )
{
  for( std::size_t i = 0 ; i < n ; ++i )
    a[ i ] /= l ;

  return a ;
}



namespace cpl {
  namespace math {

    /// \return Inner product of \a a and \a b.

    template< typename T , std::size_t n >
    inline T inner_product
    ( boost::array< T , n > const& a , boost::array< T , n > const& b )
    {

      T ret = 0 ;

      for( std::size_t i = 0 ; i != n ; ++i )
        ret += a[ i ] * b[ i ] ;

      return ret ;

    }

    /// \return \a Cross product of \a a and \a b.

    template< typename T >
    inline boost::array< T , 3 > outer_product
    ( boost::array< T , 3 > const& a , boost::array< T , 3 > const& b )
    {

      boost::array< T , 3 > ret ;

      ret[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ] ;
      ret[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ] ;
      ret[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ] ;

      return ret ;

    }


    /// \return Absolute value of \a x.

    template< typename T > inline T abs( T const& x ) {

      if( x < 0 ) return -x ;
      return x ;

    }


    /// \return The 1-norm of \a a.

    template< typename T , std::size_t n >
    inline T norm_1( boost::array< T , n > const& a ) {
      
      T ret = 0 ;
      for( typename boost::array< T , n >::const_iterator i = a.begin() ; 
          i != a.end() ; 
          ++i )
      { ret += cpl::math::abs( *i ) ; }

      return ret ;

    }


    /// \return The 2-norm of \a a.

    template< typename T , std::size_t n >
    inline T norm_2( boost::array< T , n > const& a ) {

      T ret = 0 ;
      for( typename boost::array< T , n >::const_iterator i = a.begin() ; 
          i != a.end() ; 
          ++i )
      { ret += *i * *i ; }

      return std::sqrt( ret ) ;

    }


    /// Normalize \a a (divide by the given norm function).

    template< typename T , std::size_t n , typename F >
    inline void normalize( boost::array< T , n >& a , F const& norm )
    { a /= norm( a ) ; }


    template< typename C >
    inline typename C::value_type sum( const C& c ) {

      typedef typename C::value_type T ;

      T s = T() ;
      for( typename C::const_iterator i = c.begin() ; i != c.end() ; ++i )
      { s += *i ; }
            
      return s ;

    }

  } // namespace math
} // namespace cpl

#endif // CPP_LIB_ARRAY_OPS_H
