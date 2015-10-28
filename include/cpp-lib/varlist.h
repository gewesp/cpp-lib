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
// Component: VARLIST
//

#ifndef CPP_LIB_VARLIST_H
#define CPP_LIB_VARLIST_H

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <typeinfo>
#include <istream>
#include <ostream>

#include "boost/any.hpp"

#include "cpp-lib/util.h"


namespace {

template< typename T > struct type_checker {

  static void check() {
    throw std::runtime_error( std::string( "unknown type: " )
                              + typeid( T ).name() ) ;
  }

} ;


template<> struct type_checker< double > { static void check() {} } ;
template<> struct type_checker< float  > { static void check() {} } ;
template<> struct type_checker< long   > { static void check() {} } ;
template<> struct type_checker< int    > { static void check() {} } ;

} // anonymous namespace


namespace cpl  {

namespace util {

//
// A varlist represents a list of names bound to scalar values.  The
// values can be referenced by name.  Value types supported are:
//   double
//   float
//   long
//   int
//

struct varlist {

  //
  // Make t known to the variable list.  t's lifetime must exceed the
  // lifetime of *this.
  //

  template< typename T >
  void bind( std::string const& name , T& t )
  { type_checker< T >::check() ; known[ name ] = &t ; }

  //
  // Make ts known to the varlist.  ts's lifetime must exceed the
  // lifetime of *this.
  //

  template< typename T > void vector_bind
  ( std::string const& name , std::vector< T >& ts ) ;


  //
  // Check that name refers to a T and return a reference to it.
  //

  template< typename T >
  T& reference( std::string const& name ) const ;

  template< typename T >
  T * pointer( std::string const& name ) const ;

  //
  // Return reference to the any that contains a pointer to the value
  // bound to name.
  //

  boost::any const& any_reference( std::string const& name ) const ;

  //
  // Return true iff name was bound to a variable.
  //

  bool is_defined( std::string const& name ) const
  { return known.count( name ) > 0 ; }


private:

  typedef std::map< std::string , boost::any > known_map ;
  typedef known_map::const_iterator it ;

  // The any values contain *pointers* to the variables.
  std::map< std::string , boost::any > known ;

} ;

struct stream_serializer ;

//
// Write all values known to s to os, separated by whitespace.
//

std::ostream& operator<<( std::ostream& os , stream_serializer const& s ) ;

//
// Read all values known to s from is.  CAUTION.  If the input is
// malformed, only a part of the known values may be read.
//

std::istream& operator>>( std::istream& is , stream_serializer const& s ) ;


//
// A stream_serializer holds references to variables not owned by it and
// can write them to an ostream or read them from an istream.
//

struct stream_serializer {

  //
  // Create a stream serializer to read/write the variables v, which
  // must be known by l.
  //
  // l may not change once the stream_serizlizer is created.
  //
  // Each output will first call precision( precision ) on the output stream.
  //
  // Each output will output prefix, then the bound variables, then postfix.
  //

  stream_serializer(
    varlist const& l ,
    std::vector< std::string > const& v ,
    std::string const& prefix  = "" ,
    std::string const& postfix = "" ,
    int precision = 20
  ) ;

  friend std::ostream& operator<<( std::ostream& , stream_serializer const& ) ;
  friend std::istream& operator>>( std::istream& , stream_serializer const& ) ;

private:

  // The any values contain *pointers* to the variables.
  std::vector< boost::any > vars ;

  // Strings to prepend and append at each operation.
  std::string prefix, postfix ;

  // Floating point precision for stream output.
  int precision ;

} ;


} // namespace cpl

} // namespace util


template< typename T > void
cpl::util::varlist::vector_bind
( std::string const& name , std::vector< T >& vars ) {

  for( unsigned long i = 0 ; i < vars.size() ; ++i )
  { bind( name + cpl::util::string_cast( i ) , vars[ i ] ) ; }

}


template< typename T >
T& cpl::util::varlist::reference( std::string const& name ) const {

  try {

    return *boost::any_cast< T* >( any_reference( name ) ) ;

  } catch( boost::bad_any_cast const& e ) {

    throw std::logic_error( name + ": bad type: " + e.what() ) ;

  }

}

template <typename T>
T * cpl::util::varlist::pointer(std::string const & name) const {
  if (T * const * p = boost::any_cast<T *>(&any_reference(name)))
    return *p;
  else
    return NULL;
}


#endif // CPP_LIB_VARLIST_H
