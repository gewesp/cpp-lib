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


#include <iostream>

#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include <boost/any.hpp>
#include "boost/lexical_cast.hpp"

#include "cpp-lib/registry.h"
#include "cpp-lib/util.h"

#include "cpp-lib/sys/network.h"

#include "cpp-lib/sys/logger.h"

using namespace cpl::util          ;
using namespace cpl::util::network ;


bool cpl::util::Logger::checkType( boost::any const& a ) {

  return
       boost::any_cast< double const* >( &a ) 
    || boost::any_cast< float  const* >( &a )
    || boost::any_cast< int    const* >( &a )
  ;

}


void cpl::util::Logger::setList
( std::vector< std::string > const& names ) {

  std::vector< known_map::const_iterator > new_to_log ;
  new_to_log.reserve( names.size() ) ;

  for( unsigned long i = 0 ; i < names.size() ; ++i ) {

    known_map::const_iterator const it = known.find( names[ i ] ) ;

    if( known.end() == it )
    { throw std::runtime_error( "unknown variable: " + names[ i ] ) ; }

    new_to_log.push_back( it ) ;

  } 

  to_log = new_to_log ; 

}


void cpl::util::Logger::logTo( 
  std::string const& node ,
  std::string const& service ) {
  datagram_socket::address_list_type const& d =
      resolve_datagram( node , service ) ;
  if( d.empty() ) {
    throw std::runtime_error(
      "Logger::logTo(): destination node/service resolves to zero addresses" ) ;
  }
  destination = d.at( 0 ) ;
}

void cpl::util::Logger::logFrom( 
  std::string const& service ) {
  s.reset( new datagram_socket( ipv4 , service ) ) ;
}

void cpl::util::Logger::setInterval( double const& dt_ ) {

  always_assert( dt_ == dt_ ) ;

  if( dt_ < 0 )
  { throw std::runtime_error( "Logger::setInterval: dt must be >= 0" ) ; }

  ss.reconfigure( dt_ ) ;

}


void cpl::util::Logger::bind( 
  std::string const& name ,
  std::vector< double > const * const var
) {

  for( unsigned long i = 0 ; i < var->size() ; ++i ) 
  { bind( name + boost::lexical_cast< std::string >( i ) , &var->at( i ) ) ; }

}


namespace {


template< typename T >
bool write( std::ostream& os , boost::any const& a ) {

  T const* const* const p = boost::any_cast< T const* >( &a ) ;

  if( !p ) { return false ; }

  os << **p ; 
  return true ; 

}


bool write_any( std::ostream& os , boost::any const& a ) {

  return 
       write< double >( os , a )
    || write< float  >( os , a )
    || write< int    >( os , a )
  ;

}


} // anonymous namespace


void cpl::util::Logger::log( double const& t ) {

  if( 0 == to_log.size() ) { return ; }
  if( !ss.action( t )    ) { return ; }

  std::ostringstream os ;
  os.precision( Precision ) ;

  for( 
    std::vector< known_map::const_iterator >::const_iterator it =
          to_log.begin() ;
    it != to_log.end  () ;
    ++it
  ) {    

    if( !write_any( os , ( **it ).second ) )
    { throw std::logic_error( "unknown data type in Logger::log()" ) ; }

    if( it != to_log.end() ) { os << Whitespace ; }

  }

  std::string const ss = os.str() ;

  v.assign( ss.begin() , ss.end() ) ;
  
  s->send( v.begin() , v.end() , destination ) ;

}


void cpl::util::configure
( Logger& l , registry const& reg , std::string const& basename ) {

  l.logTo( reg.get< std::string >( basename + "udp_host" ) ,
           reg.get< std::string >( basename + "udp_port" ) ) ;

  l.logFrom( reg.check_string( basename + "source_port" ) ) ;

  std::vector< std::string > const names = 
    reg.check_vector_string( basename + "variables" ) ;

  l.setList( names ) ;

  double const dt = reg.check_nonneg( basename + "dt" ) ;

  l.setInterval( dt ) ;

}
