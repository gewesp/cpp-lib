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
// Component: UTIL
//


#ifndef CPP_LIB_CONTAINER_UTIL_H
#define CPP_LIB_CONTAINER_UTIL_H

#include <array>
#include <iostream>
#include <vector>

#include <cassert>

#include "cpp-lib/util.h"

namespace cpl {

namespace util {


//
// A wrapper around std::vector<> restricted to non-resizing operations
// and safe index access.
//

template< typename T > struct rvector {

  typedef typename std::vector< T >::      iterator       iterator ;
  typedef typename std::vector< T >::const_iterator const_iterator ;

  typedef typename std::vector< T >::value_type velue_type ;
  typedef typename std::vector< T >:: size_type  size_type ;

  rvector( std::vector< T > const& v ) : v( v ) {}
  rvector( size_type const s , T const& t = T() ) : v( s , t ) {}

  T      & operator[]( size_type const i )
  { check_index( i ) ; return v[ i ] ; }
  T const& operator[]( size_type const i ) const
  { check_index( i ) ; return v[ i ] ; }

  T      & at( size_type const i )       { return v.at( i ) ; }
  T const& at( size_type const i ) const { return v.at( i ) ; }

  const_iterator begin() const { return v.begin() ; }
  const_iterator end  () const { return v.end  () ; }

        iterator begin()       { return v.begin() ; }
        iterator end  ()       { return v.end  () ; }

  size_type size() const { return v.size() ; }

private :

  void check_index( size_type const i ) const { 
    cpl::util::mark_unused( i ) ;
    assert( i < v.size() ) ; 
  }

  std::vector< T > v ;

} ;


//
// A 'capped vector' with a compile-time fixed maximum size but exposing
// push_back() etc.  Suitable for a priority_queue<> of fixed maximum size.
//
// TODO: Ensure proper destruction by pop_back()?!
//

template< typename T , std::size_t max_size > struct capped_vector {

  typedef std::array< T , max_size > nested_type ;

  typedef typename nested_type::      iterator       iterator ;
  typedef typename nested_type::const_iterator const_iterator ;

  typedef typename nested_type::value_type velue_type ;
  typedef typename nested_type:: size_type  size_type ;

  capped_vector() : size_( 0 ) { }

  T      & operator[]( size_type const i )
  { check_index( i ) ; return v[ i ] ; }
  T const& operator[]( size_type const i ) const
  { check_index( i ) ; return v[ i ] ; }

  T      & at( size_type const i )       { return v.at( i ) ; }
  T const& at( size_type const i ) const { return v.at( i ) ; }

  const_iterator begin() const { return v.begin() ; }
  const_iterator end  () const { return v.end  () ; }

        iterator begin()       { return v.begin() ; }
        iterator end  ()       { return v.end  () ; }

  std::size_t capacity() const { return max_size ; }

  bool full() const { return size() >= capacity() ; }

  bool empty() const { return 0 == size() ; }

  size_type size() const { return size_ ; }

  T const& front() const { check_index( 0 ) ; return v.front() ; }
  T      & front()       { check_index( 0 ) ; return v.front() ; }
  
  void push_back( T const& t ) {
    assert( !full() ) ;
    v[ size_ ] = t ;
    ++size_ ;
  }

  // TODO: Destructor management...
  void pop_back() { assert( !empty() ) ; --size_ ; }

private :

  void check_index( size_type const i ) const { 
    cpl::util::mark_unused( i ) ;
    assert( i < size() ) ;
  }

  std::size_t size_ ;

  nested_type v ;

} ;


namespace container {

/// Return true iff the given associative container contains the given
/// key.
template< typename C , typename key_type > 
bool contains( C const& c, key_type const& key ) {
  return c.find(key) != c.end();
}

/// Append a container to another.

/// C1 must support fast insertions at the end, C2 must be a sequence.
/// C2::value_type must be convertible to C1::value_type.
///
/// \param c2 The container to append.
/// \retval c1 The container c1 with the elements of c2 inserted at the
/// end.

template< typename C1 , typename C2 >
void append( C1& c1 , C2 const& c2 ) ;


/// Flatten a nested container.

/// C must be a container of containers, e.g. a
/// std::vector< std::vector< double > >.  The function repeatedly
/// calls cpl::util::append().
///
/// \return All elements of c joined in ``row-major'' sequence in a
/// container of C::value_type.

template< typename C >
typename C::value_type flatten( C const& c ) ;

/// Write the sequence [begin, end) to os.  If open is given as 
/// '{', '(', '[' or '<', uses the respective character to open the sequence
/// and closes it by the corresponding bracket/parenthesis at the end.
/// Separates sequence elements by the given separator.

template< class it >
void write_sequence(
    std::ostream& os,
    it const begin, 
    it const end,
    char const open = '\0',
    std::string const& separator = " ") ;

/// Write a \a cont to a file.

/// Writes the elements of cont to the named file, elements separated by
/// newline.  Assumes that the elements of cont can be written with
/// operator<<().
///
/// \param cont The container.
/// \param filename The file name.

template< class C >
void to_file( const C& cont , std::string const& filename ) ;

/// Check if iterator distance is at least d.

/// distance_at_least( i1 , i2 , d ) returns true iff
///
///   std::distance( i1 , i2 ) >= d.
///
/// The implementation, however, does _not_ use std::distance.  The
/// complexity is constant for random access iterators and O( d )
/// for other iterator categories.
///
/// Cf. Josuttis, The C++ Standard Library, Section 7.5.1 ``Writing Generic
/// Functions for Iterators''.


/// General distance_at_least.

/// \param i1 First iterator.
/// \param i2 Second iterator.
/// \param dist The distance to check.
/// \return True if and only if std::distance( i1 , i2 ) >= d.

template< typename it >
inline bool distance_at_least(
  it i1 ,
  it i2 ,
  typename std::iterator_traits< it >::difference_type dist
) {

  return distance_at_least(
    i1 ,
    i2 ,
    dist ,
    std::iterator_traits< it >::iterator_category()
  ) ;

}


/// distance_at_least() specialized for random access iterators.

template< class ran_it >
inline bool distance_at_least(
  ran_it i1 ,
  ran_it i2 ,
  typename std::iterator_traits< ran_it >::difference_type dist ,
  std::random_access_iterator_tag
) {

  return ( i2 - i1 ) >= dist ;

}


/// distance_at_least() specialized for input, forward, and bidirectional iterators.

template< class in_it >
inline bool distance_at_least(
  in_it i1 ,
  in_it i2 ,
  typename std::iterator_traits< in_it >::difference_type dist ,
  std::input_iterator_tag
) {

  typename std::iterator_traits< in_it >::difference_type d = 0 ;

  for( ; i1 != i2 ; ++i1 )

    if( ++d >= dist ) return true ;


  return false ;

}


/// \return An iterator to the next element in a sequence,
/// without modifying i.

template< typename for_it >
inline for_it next( for_it i ) { return ++i ; }


/// \return An iterator to the previous element in a sequence,
/// without modifying i.

template< typename bi_it >
inline bi_it previous( bi_it i ) { return --i ; }


//
// Advance it by at most d steps, stopping at end if it is encountered.
// d must be >= 1.
// i must come before end in the sequence.
//

template< typename it >
void safe_advance( it& i , it const& end , std::size_t const d = 1 ) ;


//
// Return i safely advanced by d steps by calling safe_advance.
//

template< typename it >
it const safe_advanced( it i , it const& end , std::size_t const d = 1 ) {

  safe_advance( i , end , d ) ;
  return i ;

}


//
// Check that 
//
//   [ begin , begin + stride , ... , end ) 
//
// is a strictly ascending sequence, i.e.  that *i < *( i + stride ) for
// all i.  If the sequence is empty, do nothing.  If the sequence is not
// strictly ascending, throw an exception.
//

template< typename it >
inline void check_strictly_ascending
( it begin , it const& end , std::size_t const stride = 1 ) ;


/// \return The iterator i advanced by d steps.

template< typename it , typename dist >
inline it advanced( it i , dist d ) {

  std::advance( i , d ) ;

  return i ;

}

//@}


//
// Return true iff all elements in [begin , end) are equal to t.
//

template< typename it , typename T >
bool all_equal( it const& begin , it const& end , T const& t ) {

  it i ;
  for( i = begin ; i != end && *i == t ; ++i ) {}

  return end == i ;

}

//
// Erase elements that satisfy a given predicate.
// C++11 code.
// http://stackoverflow.com/questions/800955/remove-if-equivalent-for-stdmap
// Container must implement a std::map<> compatible interface with
// begin(), end() and erase().
//
// Usage example:
// int test_value = 4;
// erase_if(container, [&test_value]( item_type& item ) {
//   return item.property < test_value;  // or whatever appropriate test
// });
//

template< typename C, typename P >
void erase_if( C& items, P const& predicate ) {
  for( auto it = items.begin() ; it != items.end() ; ) {
    if( predicate( *it ) ) { it = items.erase( it ) ; }
    else                   { ++it ;                   }
  }
}


} // namespace container

} // namespace util

} // namespace cpl





//
// Template definitions.
//


#include <fstream>
#include <sstream>
#include <iterator>

#include <cassert>
#include <cstdlib>

#include "cpp-lib/util.h"
#include "cpp-lib/math-util.h"


template< class it >
void cpl::util::container::write_sequence(
    std::ostream& os,
    it const begin, it const end,
    char const open, 
    std::string const& separator) {
  if (open) {
    os << open;
  }
  auto current = begin;
  while (end != current) {
    os << *current;
    ++current;
    if (end != current) {
      os << separator;
    } 
  }
  if ('{' == open) {
    os << '}';
  } else if ('[' == open) {
    os << ']';
  } else if ('(' == open) {
    os << ')';
  } else if ('<' == open) {
    os << '>';
  }
}


template< class C >
void cpl::util::container::to_file(
  const C& cont ,
  std::string const& filename
) {

  auto os = cpl::util::file::open_write( filename ) ;

  os.precision( cpl::math::DEFAULT_PRECISION ) ;

  std::copy(
    cont.begin() ,
    cont.end() ,
    std::ostream_iterator< typename C::value_type >( os , "\n" )
  ) ;

}



template< typename C1 , typename C2 >
void cpl::util::container::append( C1& c1 , C2 const& c2 ) {

  c1.insert( c1.end() , c2.begin() , c2.end() ) ;

}


template< typename C >
typename C::value_type cpl::util::container::flatten( C const& c ) {

  typename C::value_type ret ;

  for( typename C::const_iterator i = c.begin() ;
       i != c.end() ;
       ++i ) {

    append( ret , *i ) ;

  }

  return ret ;

}


namespace cpl {
namespace detail_  {

template< typename it >
inline void safe_advance(
  it& i , it const& end , std::size_t const d ,
  std::random_access_iterator_tag
) {

  assert( std::distance( i , end ) >= 0 ) ;

  if( static_cast< std::size_t >( std::distance( i , end ) ) > d )
  { i += d  ; }
  else
  { i = end ; }

}


template< typename it >
inline void safe_advance(
  it& i , it const& end , std::size_t const d ,
  std::forward_iterator_tag
) {

  for( std::size_t j = 0 ; j < d && i != end ; ++j )
  { ++i ; }

}

} // namespace detail_
} // namespace cpl


template< typename it >
inline void cpl::util::container::safe_advance
( it& i , it const& end , std::size_t const d ) { 

  assert( d >= 1 ) ;

  cpl::detail_::safe_advance
  ( i , end , d , typename std::iterator_traits< it >::iterator_category() ) ; 
  
}


template< typename it >
inline void cpl::util::container::check_strictly_ascending
( it i , it const& end , std::size_t const d ) {

  if( i == end ) { return ; }

  it j ;
  while( ( ( j = safe_advanced( i , end , d ) ) ) != end ) {

    if( *i >= *j )
    { throw std::runtime_error( "sequence not strictly ascending" ) ; }

    i = j ;

  }

}

#endif // CPP_LIB_CONTAINER_UTIL_H
