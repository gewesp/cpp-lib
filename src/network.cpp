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

#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>

#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cerrno>
#include <climits>

#include "boost/lexical_cast.hpp"
#include "boost/any.hpp"
#include "boost/cast.hpp"


#include "cpp-lib/util.h"
#include "cpp-lib/sys/network.h"

using namespace cpl::util::network ;
using namespace cpl::util          ;
using namespace cpl::detail_       ;

#include "cpp-lib/platform/net_impl.h"
#include "cpp-lib/detail/socket_lowlevel.h"


namespace {
 


void my_listen( socketfd_t const fd , int const backlog ) {

  int ret ;
  do 
  { ret = ::listen( fd , backlog ) ; } 
  while( EINTR_repeat( ret ) ) ;

  if( ret < 0 )
  { throw_socket_error( "listen" ) ; }

}


template< int type >
void my_bind( socketfd_t const fd , address< type > const& a ) {

  int ret ;
  do { ret = ::bind( fd , a.sockaddr_pointer() , a.length() ) ; } 
  while( EINTR_repeat( ret ) ) ;

  if( ret < 0 )
  { throw_socket_error( "bind" ) ; }
  
}

socketfd_t my_accept( socketfd_t const fd ) {

  // Don't care where the connection request comes from...

  socketfd_t ret ;
  do { ret = ::accept( fd , 0 , 0 ) ; } 
  while( EINTR_repeat( ret ) ) ;

  if( invalid_socket == ret ) { throw_socket_error( "accept" ) ; }

  return ret ;

}

// Returns a socket bound to the first matching address in la
// or throws.
template< int type > cpl::detail_::socket< type >
bound_socket( std::vector< address< type > > const& la ) {

  std::string err ;

  if( 0 == la.size() ) 
  { throw std::runtime_error( "must give at least one local address" ) ; }

  for( auto const& adr : la ) {

    try {

      cpl::detail_::socket< type > s( adr.family_detail_() ) ;
      my_bind( s.fd() , adr ) ;
      return s ;

    } catch( std::exception const& e ) { err = e.what() ; continue ; }

  }

  throw std::runtime_error( err ) ;

}

} // end anonymous namespace

int cpl::detail_::int_address_family( 
    cpl::util::network::address_family_type const af ) {
  switch( af ) {
    case cpl::util::network::ipv4 :
      return AF_INET ;
    case cpl::util::network::ipv6 :
      return AF_INET6 ;
    case cpl::util::network::ip_unspec :
      return AF_UNSPEC ;
    default :
      throw std::runtime_error( "unknown address family type" ) ;
  }
}

cpl::util::network::address_family_type 
cpl::detail_::from_int_address_family( int const afi ) {
  switch( afi ) {
    case AF_INET  :
      return cpl::util::network::ipv4 ;
    case AF_INET6 :
      return cpl::util::network::ipv6 ;
    case AF_UNSPEC :
      return cpl::util::network::ip_unspec ;
    default       :
      throw std::runtime_error( "unknown integer address family" ) ;
  }
}


void cpl::detail_::throw_socket_error( std::string const& msg )
{ throw std::runtime_error( msg + ": " + SOCKET_ERROR_MSG ) ; }


socketfd_t cpl::detail_::socket_resource_traits::invalid()
{ return invalid_socket ; }

bool cpl::detail_::socket_resource_traits::valid( socketfd_t const s )
{ return invalid() != s && s >= 0; }


//
// According to an FAQ, the proper sequence for closing a TCP connection
// gracefully is:
//   1. Finish sending data.
//   2. Call shutdown() with the how parameter set to 1 (SHUT_WR).
//   3. Loop on receive() until it returns 0.
//   4. Call closesocket().
//
// The connection/acceptor and {io}nstream classes strive to follow this
// pattern.
//

void cpl::detail_::socket_resource_traits::dispose
( socketfd_t const s ) { 

  if( socketclose( s ) < 0 )
  { die( "socket close(): " + SOCKET_ERROR_MSG ) ; }

} ;

////////////////////////////////////////////////////////////////////////
// Free functions
////////////////////////////////////////////////////////////////////////

cpl::util::network::address_family_type
cpl::util::network::address_family( 
    std::string const& desc , bool const allow_unspec ) {
  if        ( "ip4" == desc || "ipv4" == desc) {
    return cpl::util::network::ipv4;
  } else if ( "ip6" == desc || "ipv6" == desc ) {
    return cpl::util::network::ipv6;
  } else if ( "unspec" == desc || "any" == desc ) {
    if ( allow_unspec ) {
      return cpl::util::network::ip_unspec;
    } else {
      throw std::runtime_error(
          "need to specify address family ipv4 or ipv6" ) ;
    }
  } else {
    throw std::runtime_error( "unkown address family: " + desc ) ;
  }
}

////////////////////////////////////////////////////////////////////////
// Stream
////////////////////////////////////////////////////////////////////////

cpl::util::network::acceptor::acceptor
( address_list_type const& la , int const bl )
: s( bound_socket< SOCK_STREAM >( la ) ) ,
  local_( my_getsockname< SOCK_STREAM >( s.fd() ) )
{ my_listen( s.fd() , bl ) ; }

cpl::util::network::acceptor::acceptor
( std::string const& ls , int const bl )
: s( bound_socket< SOCK_STREAM >( resolve_stream( ls ) ) ) ,
  local_( my_getsockname< SOCK_STREAM >( s.fd() ) )
{ my_listen( s.fd() , bl ) ; }

cpl::util::network::acceptor::acceptor
( std::string const& ln , std::string const& ls , int const bl )
: s( bound_socket< SOCK_STREAM >( resolve_stream( ln , ls ) ) ) ,
  local_( my_getsockname< SOCK_STREAM >( s.fd() ) )
{ my_listen( s.fd() , bl ) ; }

std::shared_ptr<stream_socket_reader_writer>
cpl::util::network::connection::initialize( 
  address_list_type const& ra ,
  address_list_type const& la
) {

  std::string err = "connect: no suitable address combination found";

  // No local port given, use unbound socket.
  if( 0 == la.size() ) {

    for( auto const& remote : ra ) {

      try {

        auto const s = std::make_shared< stream_socket_reader_writer >( 
            remote.family_detail_() ) ;

        my_connect( s->fd() , remote ) ;
        return s;

      } catch( std::exception const& e ) { err = e.what() ; continue ; }

    }

  } else {
    // Local addresses given, find an address family match.

    for( auto const& local : la ) {

      for( auto const& remote : ra ) {

        if( local.family_detail_() != remote.family_detail_() ) 
        { err = "address families don't match" ; continue ; }

        try {

          auto const s = 
              std::make_shared< stream_socket_reader_writer >( 
                  local.family_detail_() ) ;

          my_bind   ( s->fd() , local ) ;
          my_connect( s->fd() , remote ) ;
          return s;

        } catch( std::exception const& e ) { err = e.what() ; continue ; }

      }

    }

  }

  throw std::runtime_error( err ) ;

}
 

cpl::util::network::connection::connection
( std::string const& n , std::string const& serv ) 
: s( initialize( resolve_stream( n , serv ) , address_list_type() ) ) ,
  local_( my_getsockname< SOCK_STREAM >( fd() ) ) , 
  peer_ ( my_getpeername< SOCK_STREAM >( fd() ) )
{ }


cpl::util::network::connection::connection( 
  address_list_type const& ra ,
  address_list_type const& la )
: s( initialize( ra , la ) ) ,
  local_( my_getsockname< SOCK_STREAM >( fd() ) ) , 
  peer_ ( my_getpeername< SOCK_STREAM >( fd() ) )
{ }

cpl::util::network::connection::connection
( acceptor& a )
: s( std::make_shared<stream_socket_reader_writer>(
         4711 , my_accept( a.fd() ) ) ) ,
  local_( my_getsockname< SOCK_STREAM >( fd() ) ) , 
  peer_ ( my_getpeername< SOCK_STREAM >( fd() ) )
{ }

void cpl::util::network::connection::no_delay( bool b )
{ bool_sockopt( fd() , TCP_NODELAY , b ) ; }

void cpl::util::network::connection::   send_timeout( const double t )
{ cpl::detail_::time_sockopt( fd() , SO_SNDTIMEO , t ) ; }

void cpl::util::network::connection::receive_timeout( const double t )
{ cpl::detail_::time_sockopt( fd() , SO_RCVTIMEO , t ) ; }

void cpl::util::network::connection::timeout( const double t ) {
     send_timeout( t ) ;
  receive_timeout( t ) ;
}

#if 0
// Dangerous ... not possible after shutdown().  Cf. comments in header.
cpl::util::network::stream_address
cpl::util::network::peer( cpl::util::network::onstream const& ons ) {
  return my_getpeername< SOCK_STREAM >( ons.buffer().reader_writer().fd() ) ;
}

cpl::util::network::stream_address
cpl::util::network::peer( cpl::util::network::instream const& ins ) {
  return my_getpeername< SOCK_STREAM >( ins.buffer().reader_writer().fd() ) ;
}
#endif

////////////////////////////////////////////////////////////////////////
// Datagram
////////////////////////////////////////////////////////////////////////

void cpl::util::network::datagram_socket::initialize() {
  bool_sockopt( s.fd() , SO_BROADCAST ) ;
  bool_sockopt( s.fd() , SO_REUSEADDR ) ;
}

// TODO: Use delegating constructors
cpl::util::network::datagram_socket::datagram_socket(
    cpl::util::network::address_family_type const af )
  : s( datagram_socket_reader_writer( int_address_family( af ) ) )
{ initialize() ; }

// TODO: A bit messy.  Another wrapper around my_getaddrinfo()?
cpl::util::network::datagram_socket::datagram_socket( 
    cpl::util::network::address_family_type const af ,
    std::string const& ls ) 
: s( bound_socket< SOCK_DGRAM >( 
      cpl::detail_::my_getaddrinfo< SOCK_DGRAM >( 
        NULL , /* name */
        ls.c_str() , /* service */
        int_address_family( af ) ) ) )
{ initialize() ; }

cpl::util::network::datagram_socket::datagram_socket( 
    std::string const& ln,
    std::string const& ls ) 
: s( bound_socket< SOCK_DGRAM >( resolve_datagram( ln , ls ) ) )
{ initialize() ; }

cpl::util::network::datagram_socket::datagram_socket(
  address_list_type const& la 
) : s( bound_socket< SOCK_DGRAM >( la ) )
{ initialize() ; }

void cpl::util::network::datagram_socket::connect(
    std::string const& name ,
    std::string const& service ) {
  address_list_type const& candidates = resolve_datagram( name , service ) ;

  // Look for protocol (IPv4/IPv6) match and connect.
  for ( auto const& adr : candidates ) {
    if ( local().family_detail_() == adr.family_detail_() ) {
      connect( adr ) ;
      return ;
    }
  }

  throw std::runtime_error( "datagram connect: address family mismatch" ) ;

}
