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
#include <string>
#include <map>

#include "boost/any.hpp"

#include "cpp-lib/varlist.h"
#include "cpp-lib/util.h"

using namespace cpl::util ;


cpl::util::stream_serializer::stream_serializer(
  varlist const& l ,
  std::vector< std::string > const& v ,
  std::string const& prefix  ,
  std::string const& postfix ,
  int precision

) : prefix( prefix ) , postfix( postfix ) , precision( precision ) {

  vars.reserve( v.size() ) ;

  for( unsigned long i = 0 ; i < v.size() ; ++i )
  { vars.push_back( l.any_reference( v[ i ] ) ) ; }

}


namespace {

template< typename T >
bool write_type( std::ostream& os , boost::any const& a ) {

  T const * const * const p = boost::any_cast< T* >( &a ) ;
  if( !p ) { return false ; }

  os << **p ;

  return true ;

}

template< typename T >
bool read_type( std::istream& is , boost::any const& a ) {

  T * const * const p = boost::any_cast< T* >( &a ) ;
  if( !p ) { return false ; }

  is >> **p ;

  return true ;

}

} // anonymous namespace


std::ostream& cpl::util::operator<<(
  std::ostream& os ,
  stream_serializer const& ss
) {

  os.precision( ss.precision ) ;

  os << ss.prefix ;

  for( unsigned long i = 0 ; i < ss.vars.size() ; ++i ) {

    always_assert(
         write_type< double >( os , ss.vars[ i ] )
      || write_type< float  >( os , ss.vars[ i ] )
      || write_type< long   >( os , ss.vars[ i ] )
      || write_type< int    >( os , ss.vars[ i ] )
    ) ;

    if( !os ) { break ; }
    if( i + 1 < ss.vars.size() ) { os << ' ' ; }

  }

  os << ss.postfix ;

  return os ;

}

std::istream& cpl::util::operator>>(
  std::istream& is ,
  stream_serializer const& ss
) {

  for( unsigned long i = 0 ; i < ss.vars.size() ; ++i ) {

    always_assert(
         read_type< double >( is , ss.vars[ i ] )
      || read_type< float  >( is , ss.vars[ i ] )
      || read_type< long   >( is , ss.vars[ i ] )
      || read_type< int    >( is , ss.vars[ i ] )
    ) ;

    if( !is ) { break ; }

  }

  return is ;

}


boost::any const&
cpl::util::varlist::any_reference( std::string const& name ) const {

  known_map::const_iterator const i = known.find( name ) ;
  if( known.end() == i )
  { throw std::runtime_error( "unknown variable: " + name ) ; }

  return i->second ;

}
