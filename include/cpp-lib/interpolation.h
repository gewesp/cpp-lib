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
// Component: TABLE
//

/*

This header file defines data structures and algorithms for linear 
interpolation in arbitrary dimensions.  


template< typename T > struct table ;
=====================================

A table< T > is a data type which maps a d-dimensional interval of integers
to T.  It is indexed by std::vector< size_t >, typedef'ed as index_type.
Index vectors must have size() d.

Table< T > supports operations similar to std::vector<>.  A table
can be empty(), which is equivalent to dimension() = 0.

A non-empty table t has a dimension() >= 1 (denoted by d ) and a size(),
which is an index_type indicating the one-past-the end index in each
dimension.  The elements of t can be accessed by t[ i ] where i is an
index_type with d elements and i[ j ] < size()[ j ] for 0 <= j < d.

A table can be constructed with given maximum index values in each dimension.

Empty tables can be constructed and then populated by push_back().

If empty(), push_back( t ) for t.dimension() = d - 1 ``impresses''
dimension d on *this.  size()[ d - 1 ] will be 1 and size()[ i ] will
agree with t.size()[ i ] for 0 <= i < d - 1.  *this with the d-th index
equal to zero will have t as a subtable.

If empty(), push_back( t ) for a t of type T results in *this having
dimension 1, containing exactly one element equal to t.

If dimension() = d >= 2, push_back( t ) will succeed iff 

  t.dimension() = d - 1
  t.size()[ i ] == size()[ i ] for 0 <= i < d - 1.

size()[ d - 1 ] will increase by one and *this with the d-th index equal to 
size()[ d - 1 ] - 1 will have t as a subtable.

A table< T > stores its values in a std::vector< T >.  It hands out 
the begin() and end() iterators of this value vector.  It is recommended
to use the iterators to speed up element access.

The d-dimensional structure is mapped to the value vector as follows.
The strides() function returns an index_type::const_iterator to the
beginning of a vector of length d + 1.

strides() is the ``cumulated product'' of size(), i.e. 

strides()[ 0 ] = 1
strides()[ 1 ] = size()[ 0 ]
strides()[ 2 ] = size()[ 0 ] * size()[ 1 ]
...
strides()[ d ] = size()[ 0 ] * ... * size()[ d - 1 ]

strides()[ dimension() ] is the total number of elements in the table.

Let 0 <= i < dimension().  Then, for an index j in dimension i, 

  begin()[ j * strides()[ d ] ]

traverses dimension i with all other indices fixed.


index_mapper
============

Given a strictly ascending sequence { x_0 , ... , x_{ n - 1 } }, an
index_mapper maps x to ( i , f ) such that:

i = 0     , f = 0                            if x <= x_0
i = n - 1 , f = 0                            if      x_{ n - 1 } < x 

f = ( x - x_i ) / ( x_{ i + 1 } - x_i )      if x_i < x <= x_{ i + 1 }

Postcondition: 0 <= f <= 1, i is an integer, i + f increases
piecewise linearly through the points ( x_i , i ).

The values x_0 , ... , x_{ n - 1 } are called ``breakpoints''.


template< typename T > struct hypercubic ;
template< typename T > struct simplicial ;
==========================================

These two templates wrap a table< T > t.  Their operator()() maps x \in
\Real^d to T such that it agrees with the table values for integral x
and the mapping is continuous.

T must support vector space operations.

t.size()[ i ] must be at least 2 for each i.

Before the mapping, all x[ i ] are restricted to the closed interval [ 0, 
t.size()[ i ] - 1 ].

Then, y is set to floor( x ), thus determining the grid unit cube C 
which contains x.

Then, the return value f of operator()() is computed as follows:

For hypercubic< T > it is

\sum_{ v in \{ 0 , 1 \}^d } t( y + v ) 
  \product_i( 1 - | x_i - y_i - 1 + v_i | )

simplicial< T > partitions the cube C into the d! simplices defined by

  x_pi( 0 ) <= ... <= x_pi( d - 1 )

where pi traverses the set of permutations on d elements.  (This is the
so-called Coxeter-Freudenthal-Kuhn triangulation, cf. Scott Davies.
Multidimensional triangulation and interpolation for reinforcement learning.).

x is then expressed as a convex combination of the vertices of this simplex.
Its coefficients are used to compute f as the weighted mean of the values
at the corners (which are integral and hence in the table).

Both hypercubic< T > and simplicial< T > coincide with a standard linear
interpolation for d = 1.  

operator()() is O( 2^d ) for hypercubic< T > and O( d log d ) for 
simplicial< T >.


template< typename alg > struct interpolator ;
==============================================

An interpolator< alg > maps x in \Real^d to alg::result_type (denoted by
T).  It combines d index_mapper's i_0,...,i_{d - 1} (defined by the
so-called ``breakpoints'' and a d-dimensional interpolation
algorithm a which can be hypercubic< T > or simplicial< T >.

The range of i_j must be equal to t.size()[ j ] - 1, where t is the table
wrapped by a.

operator()() returns 

  a( i_0( x[ 0 ] ) , ... , i_{ d - 1 }( x[ d - 1 ] ) ).


template< typename T > struct recursive_interpolation ;
=======================================================

This class template implements recursive multilinear interpolation 
from R^d -> T.

It holds an index_mapper m with n breakpoints and n 
recursive_interpolation< T > objects g_0 , ... , g_{ n - 1 } 
of dimension d - 1, except for d = 1 where the g_i are constants.

y = f( x_0 , ... , x_{ d - 1 } ) is defined as follows:

Let ( i , f ) be m( x_0 ).  If i = n - 1, then 

  y = g_{ n - 1 }( x_1 , ... , x_{ d - 1 } ).

If i < n - 1, then

  y = ( 1 - f ) * g_i( x_1 , ... , x_{ d - 1 } )
          + f   * g_{ i + 1 }( x_1 , ... , x_{ d - 1 } ) .


Interface class
===============

The templates hypercubic<>, simplicial<>, interpolator<> and 
recursive_interpolation<> inherit from function_n<> which defines an abstract 
interface for a function R^d -> T.


template< typename T > struct linear_interpolation ;
====================================================

This is a specialization of hypercubic< T > and simplicial< T > and
recursive_interpolation< T > to dimension one.  A linear_interpolation< T > 
is initialized by n breakpoints and the respective function values.

*/

#ifndef CPP_LIB_INTERPOLATION_H
#define CPP_LIB_INTERPOLATION_H

#include <functional>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <vector>

#include <cstdlib>
#include <cmath>

#include "boost/any.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/iterator/transform_iterator.hpp"

#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/util.h"
#include "cpp-lib/registry.h"


namespace cpl {

namespace math {

//
// Doc of this class: see header.
//

struct index_mapper : public std::unary_function< double , double > {

  //
  // Hack.  We need to use it as std container element type.
  //

  index_mapper() : xs( 1 , 0 ) {}

  //
  // Construct given { x_0 , ... , x_{ n - 1 } }.  If 
  //
  //   x_0 < ... < x_{ n - 1 }
  //
  // doesn't hold or if n == 0, an exception is thrown.
  //

  index_mapper( std::vector< double > const& xs ) ;


  //
  // Return i + f from map function.
  //

  double operator()( double const& x ) const {
    
    unsigned long i ;
    double f ;
    map( x , i , f ) ;
    return i + f ;

  }

  //
  // Compute y = i + f with integral part i and fractional part f.
  //

  void map( double const& , unsigned long& i , double& f ) const ;


  //
  // Number of elements.
  //

  std::vector< double >::size_type size() const { return xs.size() ; }


private:

  // Non-const to allow generation of assignment operator.
  std::vector< double > xs ;

} ;


//
// 1-dimensional linear interpolation.
//

template< typename T = double >
struct linear_interpolation : public std::unary_function< double , T > {

  //
  // Default constructor:  Map everything to T().
  //

  linear_interpolation() : im() , ys( 1 , T() ) {}

  //
  // Map arguments in xs to values in ys.
  //
 
  linear_interpolation(
    std::vector< double > const& xs ,
    std::vector< T      > const& ys
  ) : im( xs ) , ys( ys ) { check_sizes() ; }


  linear_interpolation( 
    index_mapper     const& im ,
    std::vector< T > const& ys
  ) : im( im ) , ys( ys ) { check_sizes() ; }


  void check_sizes() const {

    if( im.size() != ys.size() ) { 

      throw std::runtime_error( 
        "linear_interpolation:" 
        "number of fixes differs from number of breakpoints" 
      ) ;

    }

  }


  ///
  /// Map \a x[ i ] to \a y[ i ] (given in constructor) and linearly
  /// interpolate in between.
  ///
  /// Return y.front() if t < x[ i ] for all i and y.back() if t >
  /// x[ i ] for all i.
  ///
  /// If an \a x value is present more than once, the choice of the
  /// associated \a y value is implementation defined.
  ///
  /// Construction is O( n log n ) (due to sorting), lookup is O( log n )
  /// (binary search).
  ///

  T const operator()( double const& t ) const ;

private:

  index_mapper im ;
  std::vector< T > ys ;

} ;


//
// Construct a linear_interpolation<> object from a nested vector.
// \a v[ 0 ] gives the fixes, v[ 1 ] the values.  \a v must be
// a (2 x n) matrix, n >= 1.
//

linear_interpolation<> const make_linear_interpolation
( std::vector< std::vector< double > > const& v ) ;


//
// Create a linear_interpolation<> object from a registry entry of the form
// { { x1 , ... , xn } , { y1 , ... , yn } }.
//

inline linear_interpolation<> const make_linear_interpolation
( cpl::util::registry const& reg , std::string const& key ) {

  return make_linear_interpolation
         ( reg.check_vector_vector_double( key , 2 , -2 ) ) ;

}

//
// Create a linear_interpolation<> object R -> R^N from a registry entry
// of the form
// { { x1 , ... , xn } , { v1 , ... , vn } },
// where the xi are scalars and vi are N-dimensional vectors.
//

template<int N> 
linear_interpolation<cpl::matrix::vector_fixed_t<N> >
make_linear_interpolation_ndim
( cpl::util::registry const& reg , std::string const& key ) {


  typedef cpl::matrix::vector_fixed_t<N> vector_N_t;
  
  auto const v = reg.check_vector_any(key, 2);
  std::vector<double> xs;
  cpl::util::convert(v[0], xs);

  auto const n = xs.size();

  // TODO: Not very memory efficient...
  std::vector<std::vector<double> > vs;
  cpl::util::convert(v[1], vs, n, N);

  // std::vector<matrix> doesn't work for 
  // "Fixed-size vectorizable Eigen objects".
  // See http://eigen.tuxfamily.org/dox-devel/group__TopicFixedSizeVectorizable.html
  static_assert(2 != N, "2-dimension interpolation currently not supported");
  static_assert(4 != N, "4-dimension interpolation currently not supported");
  std::vector<vector_N_t> vvs;
  for (auto const& vv : vs) {
    vector_N_t tba;
    for (unsigned i = 0; i < N; ++i) {
      tba(i) = vv[i];
    }
    vvs.push_back(tba);
  }

  return linear_interpolation<vector_N_t>(xs, vvs);
}


//
// Return the array
//
// { 1 , v[ 0 ] , v[ 0 ] * v[ 1 ] , v[ 0 ] * v[ 1 ] * v[ 2 ] , ... }
//

std::vector< std::size_t >
cumulated_product( std::vector< std::size_t > const& v ) ;


//
// A d-dimensional table of data.
//

template< typename T >
struct table {

  typedef typename std::vector< std::size_t > index_type ;
  typedef typename std::vector< T >::      iterator       iterator ;
  typedef typename std::vector< T >::const_iterator const_iterator ;

  // Empty table to be filled by push_back()'s.

  table() {}

  table( index_type const& size )
  { resize( size ) ; }


  // Make empty.

  void clear() { *this = table< T >() ; }

  //
  // Resize with given max index in each dimension.
  //
  // If size().size() == 0, table will be empty.
  //

  void resize( index_type const& size ) ;

  //
  // Resize for a one-dimensional table.  Equivalent to 
  // resize( { size } ).
  //

  void resize( std::size_t const size ) ;


  T const& at( index_type const& i ) const 
  { return data_[ data_index( i ) ] ; }

  T      & at( index_type const& i )
  { return data_[ data_index( i ) ] ; }


  // Check index.

  void check_index( index_type const& i ) const {

    for( int j = 0 ; j < dimension() ; ++j )
    { assert( i[ j ] < size()[ j ] ) ; }

  }
  
  
  // Check index so that we can still add 1 in each coordinate.

  void check_index_1( index_type const& i ) const {

    for( int j = 0 ; j < dimension() ; ++j )
    { assert( size()[ j ] >= 1 &&  i[ j ] < size()[ j ] - 1 ) ; }

  }


  // 
  // Index mapping into data vector.
  //

  std::size_t data_index( index_type const& i ) const {

    assert( !empty() ) ;
    assert( i.size() == dimension() ) ;

    std::size_t ret = i[ 0 ] ;
    assert( ret < size()[ 0 ] ) ;

    for( int d = 1 ; d < dimension() ; ++d ) {

      ret += strides()[ d ] * i[ d ] ;
      assert( ret < strides()[ d + 1 ] ) ;

    }

    return ret ;

  }

  // Dimension of the table.

  std::size_t dimension() const { return size_.size() ; }
  

  // Empty table without dimensions?

  bool empty() const { return dimension() == 0 ; }


  // Maximum indices for each dimension.  Any index must be in
  // [ 0 , size() )
  index_type const& size() const { return size_ ; }

  //
  // Indexing works as follows:
  //
  // Let 0 <= d < dimension().  Then, for an integer i, 
  //
  //   begin()[ i * strides()[ d ] ]
  //
  // traverses dimension d.
  //
  // strides()[ dimension() ] is the number of elements in the table.
  //

  index_type::const_iterator const strides() const 
  { return strides_1_.begin() ; }

  // 
  // Total number of elements in the table.
  //

  std::size_t elements() const 
  { assert( !empty() ) ; return strides()[ dimension() ] ; }


  //
  // Index to elements of data vector.
  //

  const_iterator const begin() const { return data_.begin() ; }
  const_iterator const end  () const { return data_.end  () ; }


  // 
  // Append a ( d - 1 )-dimensional table at the end.  All dimensions
  // of t must match those of *this.
  //
  // If empty(), *this will be d-dimensional table with size()[ d - 1 ] = 1
  // and t as a subtable.
  //

  void push_back( table< T > const& t ) ;

  //
  // Append a T to a one-dimensional table.
  //
  // If empty(), *this will be a 1-dimensional table with size()[ 0 ] == 1 
  // and t as its element.
  //

  void push_back( T const& t ) ; 
  

private:

  index_type size_ ;

  index_type strides_1_ ;

  std::vector< T > data_ ;

} ;


//
// Convert a vector of any's to a d-dimensional table of doubles.
//

void convert( std::vector< boost::any > const& v , table< double >& t ) ;


//
// A function from R^n -> T.
//
// it's value_type must be double.
//

template< typename it , typename T = double >
struct function_n : std::binary_function< it , it , T > {
  
  virtual ~function_n() {}

  //
  // Return f( x_0 , ... , x_{ n - 1 } ) where begin points to x_0 and
  // end to x_n.
  //  
  
  virtual T const operator()( it const& begin , it const& end ) const = 0 ;

  //
  // operator()( begin , end ) calls yield defined results iff 
  //
  //   std::distance( begin , end ) == dimension.
  //

  virtual unsigned dimension() const = 0 ;

} ;


//
// An object doing hypercubic interpolation on the grid of integers.
//
// Caution.  This object is NOT thread-safe.  Use separate interpolators
// for separate threads.
//
// operator() is O( 2^d ).
//

template< typename T >
struct hypercubic : function_n< std::vector< double >::const_iterator , T > {
  
  hypercubic() {}
  
  typedef double arg_scalar_t ;
  typedef std::vector< double >::const_iterator argument_type ;

  hypercubic( table< T > const& t )
  : t( t ) , x( t.dimension() ) {}

  T const operator()
  ( argument_type const& begin , argument_type const& end ) const ;

  T const operator()( std::vector< double > const& x ) const 
  { return ( *this )( x.begin() , x.end() ) ; }

  table< T > const& data() const { return t ; }

  unsigned dimension() const { return t.dimension() ; }

private:

  table< T > t ;
  
  mutable std::vector< double > x ;

} ;


//
// An object doing simplicial interpolation on the grid of integers.
//
// operator() is O( d log d ).
//
// Caution.  This object is NOT thread-safe.  Use separate interpolators
// for separate threads.
//

template< typename T >
struct simplicial : function_n< std::vector< double >::const_iterator , T > {
  
  typedef double arg_scalar_t ;
  typedef std::vector< double >::const_iterator argument_type ;
  
  simplicial() {}

  simplicial( table< T > const& t )
  : t( t ) , x( t.dimension() ) {}
  
  T const operator()
  ( argument_type const& begin , argument_type const& end ) const ;

  T const operator()( std::vector< double > const& x ) const 
  { return ( *this )( x.begin() , x.end() ) ; }
  
  table< T > const& data() const { return t ; }
  
  unsigned dimension() const { return t.dimension() ; }

private:

  table< T > t ;
  
  typedef std::vector< std::pair< double , std::size_t > > pair_vector ;
  mutable pair_vector x ;

} ;


//
// A multidimensional interpolator object with breakpoints in each
// dimension using an arbitrary grid algorithm (like hypercubic or
// simplicial).
//
// Caution.  This object uses internal state between calls to const
// methods.  Hence it is not multithread safe.  Use multiple objects for
// different threads.
//

template< typename alg >
struct interpolator 
: function_n
  < std::vector< double >::const_iterator , typename alg::result_type > {
  
  typedef double arg_scalar_t ;
  typedef std::vector< double >::const_iterator argument_type ;

  interpolator() {}

  //
  // Construct an interpolator object using algorithm alg (which must be
  // hypercubic or simplicial) on a table t and with breakpoints b.
  //

  interpolator( 
    table< typename alg::result_type >   const& t ,
    std::vector< std::vector< double > > const& b
  ) ;

  typename alg::result_type const 
  operator()( argument_type const& begin , argument_type const& end ) const ;

  typename alg::result_type const 
  operator()( std::vector< double > const& x ) const 
  { return ( *this )( x.begin() , x.end() ) ; }

  unsigned dimension() const { return engine.dimension() ; }

private:

  alg engine ;
  std::vector< index_mapper > im ;
  
  mutable std::vector< double > x ;

} ;


//
// Create an interpolator object from given breakpoint and value table.
//
// Value type must be alg::result_type.
//

template< typename alg >
interpolator< alg > const
make_interpolator( boost::any const& breakpoints , boost::any const& table ) ;


//
// Create an interpolator object from given breakpoint table b and value 
// table v as defined under given key in given registry in the form 
// { b , v }.
//
// Value type must be alg::result_type.
//

template< typename alg >
interpolator< alg > const
make_interpolator( cpl::util::registry const& , std::string const& key ) ;


//
// A function mapping vectors of arg_scalar_t to T by recursive
// linear interpolation.
//

template< typename T = double >
struct recursive_interpolation 
: function_n< std::vector< double >::const_iterator , T > {
  
  typedef double arg_scalar_t ;
  typedef std::vector< double >::const_iterator argument_type ;

  //
  // Recursively initialize from expression of the form 
  //
  //   { { b_0 ... b_{ n - 1 } } g_0 ... g_{ n - 1 } }
  //

  recursive_interpolation( boost::any const& ) ;

  virtual T const operator()( 
    argument_type const& begin ,
    argument_type const& end
  ) const ;

  T const operator()( std::vector< arg_scalar_t > const& x ) const
  { return ( *this )( x.begin() , x.end() ) ; }

  virtual unsigned dimension() const { return dimension_ ; }

private:

  unsigned dimension_ ;

  index_mapper im ;

  linear_interpolation< T > f ;
  std::vector< boost::shared_ptr< function_n< argument_type , T > > > ff ;

} ;


} // end namespace math

} // end namespace cpl


//
// Template definitions.
//

template< typename T >
T const cpl::math::linear_interpolation< T >::operator()
( double const& t ) const {

  assert( t == t ) ;
  
  assert( im.size() >= 1 ) ;
  assert( im.size() == ys.size() ) ;

  unsigned long i ;
  double f ;

  im.map( t , i , f ) ;

  assert( i < im.size() ) ;

  if( im.size() - 1 == i ) { return ys.back() ; } 

  assert( i < im.size() - 1 ) ;

  return ( 1 - f ) * ys[ i ]  + f * ys[ i + 1 ] ;

}


namespace {


template< typename T >
T const hypercubic_recurse( 
  cpl::math::table< T >            const& t ,
  std::vector< double >::const_iterator const  x ,
  unsigned                              const  d ,
  std::size_t                           const  i
) {

  assert( d < t.dimension() ) ;

  if( d == 0 ) {

    assert( t.size()[ 0 ] >= 2 ) ;

    return
      ( 1 - x[ 0 ] ) * t.begin()[ i     ] 
        +   x[ 0 ]   * t.begin()[ i + 1 ] ; 

  }

  assert( d >= 1 ) ;
  assert( d < t.size().size() ) ;
  assert( t.size()[ d ] >= 2 ) ;

  return 
      ( 1 - x[ d ] ) * hypercubic_recurse
               ( t , x , d - 1 , i                    )
    +       x[ d ]   * hypercubic_recurse
               ( t , x , d - 1 , i + t.strides()[ d ] ) 
  ;

}


// 
// Write the fractional part of the y's to the x's and return the base
// index of y into t's data table.
//
// x must be an output iterator.
//

template< typename T , typename o_it >
std::size_t remainder( 
  cpl::math::table< T >            const& t    ,
  std::vector< double >::const_iterator const& y    , 
  std::vector< double >::const_iterator const& end  , 
  o_it                                         x 
) {

  assert( 
    static_cast< unsigned long >( std::distance( y , end ) ) == t.dimension()
  ) ;

  std::size_t ret = 0 ;

  for( unsigned d = 0 ; d < t.dimension() ; ++d ) {

    //
    // ``Special'' floor because we need to treat closed intervals as
    // well.  Remainder may be up do 1 + epsilon.
    //
    // The resulting index is so that we can safely advance by one into
    // each dimension.
    //

    assert( 2 <= t.size()[ d ] ) ;
    assert( 0 <= y[ d ] ) ;
    std::size_t i = static_cast< std::size_t >( std::floor( y[ d ] ) ) ;
    assert( i < t.size()[ d ] ) ;
    if( i == t.size()[ d ] - 1 ) { --i ; }
    assert(      i < t.size()[ d ] - 1 ) ;

    *x = y[ d ] - i ;
    ++x ;

    ret += t.strides()[ d ] * i ;
    assert( ret < t.elements() ) ;

  }

  return ret ;

}


} // anonymous namespace


template< typename T >
T const cpl::math::hypercubic< T >::operator()
( argument_type const& begin , argument_type const& end ) const {

  assert( t.dimension() >= 1 ) ;
  assert( 
    static_cast< unsigned long >( std::distance( begin , end ) ) 
    == t.dimension() 
  ) ;
  assert( x.size() == t.dimension() ) ;

  // Get x in [ 0 , 1 )^d
  std::size_t const i = remainder( t , begin , end , x.begin() ) ;

  return hypercubic_recurse
  ( t , x.begin() , t.dimension() - 1 , i ) ;

}


//
// Algorithm:
//
//   f( x ) = l_0 f( c^0 ) + ... + l_n f( c^n )
//
//          =   x_pi( 1 ) f( c^0 ) 
//            + sum_{ 1 <= i <= n - 1 } x_pi( i + 1 ) - x_pi( i ) ) f( c^i )
//            + ( 1 - x_pi( n ) ) f( c^n )
//
// where x_pi( 1 ) <= ... <= x_pi( n ) is the sorted sequence of x (pi
// can be obtained by sorting 0 , ... , n - 1 together with x).
//
// The c^i are defined as
//
//    c^i = \sum_{ k = i + 1 }^n e_pi( k ),
//
//  hence the algorithm runs backwards starting with
//
//    c^n = 0 
//
//  and 
//
//    c^i = c^{ i + 1 } + e_pi( i + 1 ).
//

template< typename T >
T const cpl::math::simplicial< T >::operator()
( argument_type const& begin , argument_type const& end ) const {
  
  using namespace cpl::detail_ ;

  assert( t.dimension() >= 1 ) ;
  assert( 
    static_cast< unsigned long >( std::distance( begin , end ) ) 
    == t.dimension() 
  ) ;

  // Get x in [ 0 , 1 )^d
  typedef cpl::util::pair_access_first< double , std::size_t > my_paf ;
  boost::transform_iterator< my_paf , pair_vector::iterator > 
    it( x.begin() , my_paf() ) ;
  typename cpl::math::table< T >::const_iterator val_it = 
    t.begin() + remainder( t , begin , end , it ) ;

  // Write indices for permutation
  for( std::size_t j = 0 ; j < t.dimension() ; ++j )
  { x[ j ].second = j ; }

  std::sort
  ( x.begin() , x.end() , 
    cpl::util::pair_less_first< double , std::size_t >() ) ;

  assert( val_it < t.end() ) ;
  T ret = ( 1. - x.back().first ) * *val_it ;

  //
  // Loop is empty iff dimension() == 1.
  //

  for( std::size_t j = t.dimension() - 1 ; j > 0 ; --j ) {
    
    std::size_t const step = t.strides()[ x[ j ].second ] ;
    assert( step > 0 ) ;
    val_it += step ;

    assert( val_it < t.end() ) ;
    ret += 
      ( x[ j ].first - x[ j - 1 ].first ) * *val_it ;
    
  }

  assert( t.strides()[ x.front().second ] > 0 ) ;
  val_it += t.strides()[ x.front().second ] ;

  assert( val_it < t.end() ) ;
  ret += x.front().first * *val_it ;

  return ret ;

}


template< typename T >
void cpl::math::table< T >::resize( std::size_t const size ) {

  size_.resize( 1 ) ;
  size_[ 0 ] = size ;
  data_.resize( size ) ;
  strides_1_ = cumulated_product( size_ ) ;

}


template< typename T >
void cpl::math::table< T >::resize( index_type const& size ) {

  size_ = size ;
  strides_1_ = cumulated_product( size ) ;
  data_.resize( strides_1_.back() ) ;

}


template< typename T >
void cpl::math::table< T >::push_back
( cpl::math::table< T > const& t ) {

  if( t.empty() ) { 
    throw std::runtime_error
    ( "table<>::push_back: attempt to push empty table" ) ; 
  }

  if( dimension() == 1 )
  { throw std::runtime_error( "table<>::push_back: dimension mismatch" ) ; }

  if( empty() ) {

    // Impress dimension of t, have *this have one more dimension.
    index_type new_size = t.size() ;
    new_size.push_back( 0 ) ;
    resize( new_size ) ;

  }

  // These two asserts should be guaranteed by the above checks.
  assert( dimension() >= 2 ) ;
  assert( t.dimension() >= 1 ) ;

  if( t.dimension() + 1 != dimension() )
  { throw std::runtime_error( "table<>::push_back: dimension mismatch" ) ; }
  assert( t.dimension() + 1 == dimension() ) ;

  for( unsigned d = 0 ; d < t.dimension() ; ++d ) {

    if( size()[ d ] != t.size()[ d ] ) 
    { throw std::runtime_error( "table<>::push_back: dimension mismatch" ) ; }

  }

  data_.insert( data_.end() , t.begin() , t.begin() + t.elements() ) ;

  strides_1_[ dimension() ] += strides_1_[ dimension() - 1 ] ;
  ++size_[ dimension() - 1 ] ;

  assert( elements() == data_.size() ) ;

}


template< typename T >
void cpl::math::table< T >::push_back
( T const& t ) {

  if( dimension() == 0 ) { resize( 0 ) ; assert( dimension() == 1 ) ; }

  if( dimension() != 1 ) 
  { throw std::runtime_error( "table<>::push_back: dimension mismatch" ) ; }

  data_.push_back( t ) ;
  
  assert( strides_1_.size() == 2 ) ;
  assert( size().size() == 1 ) ;

  ++strides_1_[ 1 ] ;
  ++size_     [ 0 ] ;

  assert( elements() == data_.size() ) ;

}


template< typename alg >
cpl::math::interpolator< alg >::interpolator(
  cpl::math::table< typename alg::result_type > const& t ,
  std::vector< std::vector< double > >               const& b
) : engine( t ) {
  
  x.resize( engine.data().dimension() ) ;

  if( engine.data().dimension() != b.size() )
  { throw std::runtime_error( "interpolator: dimension mismatch" ) ; }

  im.reserve( engine.data().dimension() ) ;

  for( std::size_t d = 0 ; d < engine.data().dimension() ; ++d ) {

    if( engine.data().size()[ d ] < 2 ) { 

      throw std::runtime_error( 
      "interpolator: dimension " 
    + boost::lexical_cast< std::string >( d + 1 )
    + ": must be at least 2"
      ) ;

    }

    if( engine.data().size()[ d ] != b[ d ].size() ) {

      throw std::runtime_error( 
      "interpolator: dimension " 
    + boost::lexical_cast< std::string >( d + 1 )
    + ": doesn't match number of breakpoints"
      ) ;

    }

    try { im.push_back( cpl::math::index_mapper( b[ d ] ) ) ; }
    catch( std::runtime_error const& e ) {

      throw std::runtime_error( 
      "interpolator: dimension " 
    + boost::lexical_cast< std::string >( d + 1 )
    + ": " + e.what() 
      ) ;

    }

  }

}


template< typename alg >
typename alg::result_type const 
cpl::math::interpolator< alg >::operator()
( argument_type const& y , argument_type const& end ) const {

  assert
  ( static_cast< unsigned long >( std::distance( y , end ) ) == x.size() ) ;
  assert( im.size()                == x.size() ) ;

  for( std::size_t i = 0 ; i < engine.data().dimension() ; ++i ) {

    x[ i ] = im[ i ]( y[ i ] ) ;

    assert( 0 <= x[ i ] ) ;

  }

  return engine( x ) ;

}


template< typename T >
cpl::math::recursive_interpolation< T >::recursive_interpolation
( boost::any const& a ) {

  try {

  std::vector< boost::any > const& va = 
    cpl::util::convert< std::vector< boost::any > >( a ) ;

  unsigned long n = va.size() ;

  if( n < 2 ) 
  { throw std::runtime_error( "need { breakpoints } and values..." ) ; }

  std::vector< double > xs ;
  cpl::util::convert( va.at( 0 ) , xs ) ;

  if( xs.size() != n - 1 ) {

    throw std::runtime_error
    ( "number of breakpoints must match number of subinterpolations" ) ;

  }

  std::vector< double > ys( n - 1 ) ;

  bool recurse = false ;

  for( unsigned long i = 0 ; i < ys.size() ; ++i ) {

    double const* const p = 
      boost::any_cast< double >( &va.at( i + 1 ) ) ;

    if( !p ) { recurse = true ; break ; }

    ys.at( i ) = *p ;

  }

  if( !recurse ) {

    dimension_ = 1 ;

    f = cpl::math::linear_interpolation< T >( xs , ys ) ;

  } else {

    ff.resize( n - 1 ) ;

    im = cpl::math::index_mapper( xs ) ;

    for( unsigned long i = 0 ; i < ff.size() ; ++i ) 
    { ff.at( i ).reset( new recursive_interpolation< T >( va.at( i + 1 ) ) ) ; }

    dimension_ = ff.at( 0 )->dimension() + 1 ;

    for( unsigned long i = 0 ; i < ff.size() ; ++i ) {

      if( ff.at( i )->dimension() + 1 != dimension() ) {

        throw std::runtime_error( 
            "subinterpolation #"
          + boost::lexical_cast< std::string >( i ) 
          + " has wrong dimension"
        ) ;

      }

    }

  }

  } catch( std::runtime_error const& e ) {

    throw std::runtime_error( 
        std::string( "cannot initialize recursive_interpolation: " )
      + e.what() 
    ) ;

  }
 
}


template< typename T >
T const cpl::math::recursive_interpolation< T >::operator()( 
  argument_type const& begin ,
  argument_type const& end
) const {

  assert( 
    static_cast< unsigned long >( std::distance( begin , end ) ) == dimension()
  ) ;

  if( 1 == dimension() ) { return f( *begin ) ; }

  arg_scalar_t const& x1 = *begin ;

  assert( x1 == x1 ) ;

  assert( im.size() >= 1 ) ;
  assert( im.size() == ff.size() ) ;

  unsigned long i ;
  arg_scalar_t h ;

  im.map( x1 , i , h ) ;

  assert( i < im.size() ) ;

  if( im.size() - 1 == i ) { return ( *ff.back() )( begin + 1 , end ) ; }

  assert( i < im.size() - 1 ) ;

  return 
      ( 1 - h ) * ( *ff[ i     ] )( begin + 1 , end )
    +       h   * ( *ff[ i + 1 ] )( begin + 1 , end ) 
  ;

}


template< typename alg >
cpl::math::interpolator< alg > const
cpl::math::make_interpolator
( boost::any const& breakpoints , boost::any const& values ) {
    
  cpl::math::table< typename alg::result_type > t ;
  std::vector< std::vector< double > > bp ;

  try {

    std::vector< boost::any > const &va =
      cpl::util::convert< std::vector< boost::any > >( values ) ;
    
    convert( va , t ) ;

  } catch( std::runtime_error const& e ) {

    throw std::runtime_error( 
        "make_interpolator: bad value table format: " 
      + std::string( e.what() ) 
    ) ;

  }

  try { 
    
    std::vector< boost::any > const& va =
      cpl::util::convert< std::vector< boost::any > >( breakpoints ) ;

    std::vector< double > bp1 ;

    try { 

      // 
      // Check for breakpoint table format { x1 ... xn } (1-d).
      //

      cpl::util::convert( va , bp1 ) ;
      bp.push_back( bp1 ) ;

    } catch( std::runtime_error const& ) {
      
      // 
      // Ignore; try other format { { x1 ... xn } ... }
      //

      cpl::util::convert( breakpoints , bp ) ;

    }
 
  } 
  catch( std::runtime_error const& e ) {
    
    throw std::runtime_error( 
        "make_interpolator: bad breakpoint table format: " 
      + std::string( e.what() ) 
    ) ;

  }

  return cpl::math::interpolator< alg >( t , bp ) ;

}


template< typename alg >
cpl::math::interpolator< alg > const
cpl::math::make_interpolator
( cpl::util::registry const& reg , std::string const& key ) {

  std::vector< boost::any > const& v = reg.check_vector_any( key , 2 ) ;

  try {

  cpl::math::interpolator< alg > const ret =
  cpl::math::make_interpolator< alg >( v[ 0 ] , v[ 1 ] ) ;
  
  return ret ;

  } catch( std::exception const& e ) { 
    
    throw std::runtime_error
    ( std::string( e.what() ) + ": " + reg.key_defined_at( key ) ) ; 

  }

}

#endif // CPP_LIB_INTERPOLATION_H
