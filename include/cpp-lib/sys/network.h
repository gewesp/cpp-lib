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
// Component: NETWORK
//


#ifndef CPP_LIB_NETWORK_H
#define CPP_LIB_NETWORK_H

#include <vector>
#include <istream>
#include <ostream>
#include <streambuf>
#include <stdexcept>
#include <exception>
#include <limits>
#include <iostream>
#include <iterator>

#include <cstdlib>

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>

#include "cpp-lib/util.h"


#include "cpp-lib/detail/socket_lowlevel.h"
#include "cpp-lib/platform/wrappers.h"

namespace cpl {

namespace util {

namespace network {

struct acceptor   ;
struct connection ;

} // namespace network

} // namespace util


namespace detail_ {


struct socket_resource_traits {

  static socketfd_t invalid() ;
  static bool valid  ( socketfd_t const s ) ;
  static void dispose( socketfd_t const s ) ;
  
} ;


typedef 
cpl::util::auto_resource< socketfd_t , socket_resource_traits > 
auto_socket_resource ;


inline int check_socktype( int const type ) 
{ always_assert( SOCK_DGRAM == type || SOCK_STREAM == type ) ; return type ; }

inline int check_family( int const family ) 
{ always_assert( AF_INET == family || AF_INET6 == family ) ; return family ; }
  

// A socket template with some intelligence, e.g. it checks
// its parameters.  Suitable for stream and datagram, specialized
// below.  Holds an auto_resource for the socket descriptor.
// Implements the 'reader_writer' abstraction.
template< int type > struct socket {

  // Just to be sure...
  socket& operator=( const socket&  ) = delete;
  socket           ( const socket&  ) = delete;

  // Default move construction: Moves contents
  // Move assignent deleted for the time being
  socket           ( socket&&      ) = default ;
  socket& operator=( socket&&      ) = delete;

  socketfd_t fd() const { return fd_.get() ; }

  bool valid() const { return fd_.valid() ; }

  void setup() const {
    
    if( !fd_.valid() )
    { throw_socket_error( "socket" ) ; }

    if( SOCK_STREAM == type ) 
    { setup_stream_socket( fd() ) ; }

  }
    

  // Constructor overload with a dummy parameter to distinguish
  // from constructor with family.  Used if the file descriptor
  // is already known (e.g., accept).
  socket( int const /* ignored */, socketfd_t const fd ) : fd_( fd )
  { setup() ; }

  socket( int const family )
  : fd_( ::socket( check_family( family ) , check_socktype( type ) , 0 ) ) 
  { setup() ; }

  long read ( char      * buf , long n ) ;
  long write( char const* buf , long n ) ;

  // No-ops for DGRAM sockets
  void shutdown_read () 
  { if ( SOCK_STREAM == type ) { socket_shutdown_read ( fd() ) ; } }
  void shutdown_write()
  { if ( SOCK_STREAM == type ) { socket_shutdown_write( fd() ) ; } }

private:

  // This wrapper around the handle takes care of move semantics.
  auto_socket_resource fd_ ;

} ;


typedef socket< SOCK_DGRAM  > datagram_socket_reader_writer ;
typedef socket< SOCK_STREAM >   stream_socket_reader_writer ;


// A safety wrapper around struct sockaddr_storage, templatized on
// SOCK_DGRAM/SOCK_STREAM.
template< int type >
struct address {

  // Initialize to ipv4:0.0.0.0:0
  address() : addrlen( maxlength() ) {
    std::memset( &addr , 0 , sizeof( addr ) ) ;
    sockaddr_pointer()->sa_family = AF_INET;
  }

  address( sockaddr_storage const& addr , socklen_t addrlen )
  : addr( addr ) , addrlen( addrlen ) {}

  // Numeric form.
  std::string const host() const ;
  std::string const port() const ;

  // These try reverse lookup and throw if it fails.
  std::string const host_name() const ;
  std::string const port_name() const ;

  std::string const fqdn() const ;

  bool dgram() const { return SOCK_DGRAM == type ; }

  // sa_family_t family() const { return sockaddr_pointer()->sa_family ; }
  unsigned  family() const { return sockaddr_pointer()->sa_family ; }
  socklen_t length() const { return addrlen ; }

  socklen_t maxlength() const { return sizeof( sockaddr_storage ) ; }
  void set_maxlength() { addrlen = maxlength() ; }

  // Accessors for the low-level stuff...
  sockaddr* sockaddr_pointer() 
  { return reinterpret_cast< sockaddr* >( &addr ) ; }

  sockaddr const* sockaddr_pointer() const
  { return reinterpret_cast< sockaddr const* >( &addr ) ; }

  sockaddr_in  const& as_sockaddr_in () const
  { return *reinterpret_cast< sockaddr_in  const* >( &addr ) ; }

  sockaddr_in6 const& as_sockaddr_in6() const
  { return *reinterpret_cast< sockaddr_in6 const* >( &addr ) ; }


  socklen_t const* socklen_pointer() const { return &addrlen ; }
  socklen_t      * socklen_pointer()       { return &addrlen ; }

private:

  // From struct addrinfo:
  sockaddr_storage addr    ;
  socklen_t        addrlen ;

} ;


// TODO: Naive implementation, maybe take sin6_flowinfo etc. into account?
// http://linux.die.net/man/7/ipv6
template< int type >
bool operator!=( address< type > const& a1 , address< type > const& a2 ) {
  if( a1.family() != a2.family() ) {
    return true;
  }

  if( AF_INET == a1.family() ) {
    return    a1.as_sockaddr_in().sin_port
           != a2.as_sockaddr_in().sin_port
        ||    a1.as_sockaddr_in().sin_addr.s_addr
           != a2.as_sockaddr_in().sin_addr.s_addr
    ;
  } else {
    return    a1.as_sockaddr_in6().sin6_port
           != a2.as_sockaddr_in6().sin6_port
        ||    !cpl::util::mem_equal(a1.as_sockaddr_in6().sin6_addr,
                                    a2.as_sockaddr_in6().sin6_addr)
    ;
  }
}

template< int type >
bool operator==( address< type > const& a1 , address< type > const& a2 ) {
  return !(a1 != a2) ;
}

// Output operator, writing the numeric address and port.
// IPv6 addresses are surrounded by square brackets as per
// http://en.wikipedia.org/wiki/IPv6_address
template< int type >
std::ostream& operator<<( std::ostream& os , address< type > const& a ) {

  if( AF_INET6 == a.family() ) {
    os << '[';
  }
  os << a.host() ;
  if( AF_INET6 == a.family() ) {
    os << ']';
  }
  os << ":" << a.port() ; 

  return os ;
}


template< int type >
std::vector< address< type > >
my_getaddrinfo( char const* n , char const* s , 
                int family_hint = AF_UNSPEC ) ;


template< int type >
long my_sendto
( socketfd_t const fd , address< type > const& a , char const* p , long n ) {

  long ret ;
  do { ret = ::sendto( fd , p , n , 0 , a.sockaddr_pointer() , a.length() ) ; }
  while( cpl::detail_::EINTR_repeat( ret ) ) ;

  return ret ;

}

inline long my_send( socketfd_t const fd , char const* p , long n ) {

  long ret ;
  do { ret = ::send( fd , p , n , 0 ) ; }
  while( cpl::detail_::EINTR_repeat( ret ) ) ;

  return ret ;

}




template< int type >
std::string const
my_getnameinfo(
  address< type >  const& a ,
  bool             const node    ,  // node or service?
  bool             const numeric ,  // numeric or name?
  bool             const fqdn       // fqdn (for node)?
) ;


template< int type >
void my_connect( socketfd_t const fd , address< type > const& a ) {

  int ret ;
  do 
  { ret = ::connect( fd , a.sockaddr_pointer() , a.length() ) ; }
  while( EINTR_repeat( ret ) ) ;

  if( ret < 0 )
  { throw_socket_error( "connect" ) ; }

}

} // namespace detail_


namespace util {

namespace network {

// Avoid overlap of enum with port numbers...
enum address_family_type { ipv4 = 1000123 , ipv6 = 1000343 } ;

// Returns ipv4 for "ipv4", "ip4" etc.
// Throws if description cannot be recognized.
address_family_type address_family( std::string const& description ) ;

////////////////////////////////////////////////////////////////////////
// cpp-lib specific functionality.
////////////////////////////////////////////////////////////////////////

//
// Check n and throw an exception if it is not a valid port number.
//

template< typename T >
void check_port( T const& n ) {
  
  if( n < 0 || n > 65535 )
  { throw std::runtime_error( "TCP/UDP port number out of range" ) ; }

}


////////////////////////////////////////////////////////////////////////
// Proposed standard interface.
////////////////////////////////////////////////////////////////////////

typedef cpl::detail_::address< SOCK_STREAM >   stream_address ;
typedef cpl::detail_::address< SOCK_DGRAM  > datagram_address ;

typedef std::vector<   stream_address >   stream_address_list ;
typedef std::vector< datagram_address > datagram_address_list ;


inline stream_address_list const resolve_stream
( std::string const& n , std::string const& s ) { 

  return cpl::detail_::my_getaddrinfo< SOCK_STREAM >( n.c_str() , s.c_str() ) ;

}

inline datagram_address_list const resolve_datagram
( std::string const& n , std::string const& s ) {

  return cpl::detail_::my_getaddrinfo< SOCK_DGRAM >( n.c_str() , s.c_str() ) ;

}

inline stream_address_list const resolve_stream
( std::string const& s ) {

  return cpl::detail_::my_getaddrinfo< SOCK_STREAM >( 0 , s.c_str() ) ;

}

inline datagram_address_list const resolve_datagram
( std::string const& s ) {
  
  return cpl::detail_::my_getaddrinfo< SOCK_DGRAM >( 0 , s.c_str() ) ;

}


////////////////////////////////////////////////////////////////////////
// Datagram communications.
//
// TODO: The API is still in flux and currently not in line with N1925.
// TODO: Implement smart decision on which address from resolver to
// use.
// TODO: Thread safety is in a mess.  Remove state from the class
// to make it thread-safe (e.g., source_).  Seriously.
////////////////////////////////////////////////////////////////////////

struct datagram_socket {

  // Types

  typedef datagram_address      address_type      ;
  typedef datagram_address_list address_list_type ;

  typedef unsigned long size_type ;

  // Constructors

  // Create unbound IPv4 or IPv6 socket.
  // User code should use e.g. datagram_socket sock(ipv4);
  datagram_socket( address_family_type ) ;

  // Binds to local service ls.
  datagram_socket( address_family_type , std::string const& ls ) ;

  // Bind to local name and service
  // Use ("0.0.0.0", ls) for IPv4
  // Use ("::1", ls) for IPv6
  datagram_socket( std::string const& ln, std::string const& ls ) ;

  // Bind to first suitable of the given local addresses
  datagram_socket( address_list_type const& la ) ;

  static size_type timeout() 
  { return std::numeric_limits< size_type >::max() ; }

  static size_type default_size() { return 65536 ; }

  // Connect to the given name/service, use IPv4 or IPv6 according
  // to own socket family.
  void connect( std::string const& name , std::string const& service ) ;

  // Connect to peer address.  Packets sent by send() will
  // be sent to this address.
  void connect( address_type const& destination ) {
    my_connect( s.fd() , destination ) ;
    peer_ = destination ;
    connected_ = true ;
  }

  // Receive a packet with timeout t [s].
  // t: Timeout.  If >= 0, waits for a maximum of t seconds (even zero).
  // If t < 0, blocks.
  // Returns: timeout() on timeout, or the number of characters written.
  // n: Maximum size to receive.
  template< typename for_it >
  size_type receive( 
    for_it const& begin , 
    double const& t = -1 ,
    size_type n = default_size()
  ) ;

  // Send overloads.
  // Connection refused error is ignored.

  // Send to address given in connect().
  template< typename for_it >
  void send( for_it const& begin, for_it const& end ) ;

  // Send to given destination.
  template< typename for_it >
  void send( 
    for_it       const& begin       ,
    for_it       const& end         ,
    address_type const& destination
  ) ;
 
  // Send to given node/service.
  template< typename for_it >
  void send( 
    for_it      const& begin   ,
    for_it      const& end     ,
    std::string const& node    ,
    std::string const& service
  ) ;


  // Observers

  // Address of last packet received
  // TODO: This isn't thread safe!
  address_type const& source() const { return source_ ; }

  // Local address, for bound sockets only
  address_type const& local () const { 
    if( !bound() ) {
      throw std::runtime_error( "unbound datagram socket" ) ;
    }
    return  local_ ; 
  }

  // Remote address, for connected sockets only
  address_type const& peer() const {
    if( !connected() ) {
      throw std::runtime_error( "unconnected datagram socket" ) ;
    }
    return peer_ ; 
  }

  // Is socket bound?
  // bool bound    () const { return bound_     ; }
  bool bound() const { return true ; }


  // Is socket connected?
  bool connected() const { return connected_ ; }

private:

  // Sets up local_ and enables broadcasting.
  void initialize();

  cpl::detail_::datagram_socket_reader_writer s ;
  cpl::detail_::socketfd_t fd() const { return s.fd() ; }

  address_type source_ ;
  address_type  local_ ;

  // Remote address for connected sockets.
  address_type peer_ ;

  // Socket is bound, local() valid
  // bool bound_;

  // Socket is connected, can use send(begin, end)
  bool connected_;

} ;


////////////////////////////////////////////////////////////////////////
// Stream communications.
////////////////////////////////////////////////////////////////////////

//
// istreambuf<> and ostreambuf<> specializations for stream (TCP)
// sockets.
//

typedef cpl::util::istreambuf< cpl::detail_::stream_socket_reader_writer > 
instreambuf ;

typedef cpl::util::ostreambuf< cpl::detail_::stream_socket_reader_writer > 
onstreambuf ;

} // namespace network

namespace file {

template<> struct buffer_maker_traits< ::cpl::util::network::connection > {
  typedef cpl::util::network::instreambuf istreambuf_type ;
  typedef cpl::util::network::onstreambuf ostreambuf_type ;
} ;

} // namespace file

namespace network {

struct connection : cpl::util::file::buffer_maker< connection > {

  // Move, but don't copy
  connection(connection&&) = default;
  connection& operator=(connection&&) = default;

  connection(connection const&) = delete;
  connection& operator=(connection const&) = delete;
  
  typedef stream_address      address_type      ;
  typedef stream_address_list address_list_type ;

  connection( 
    address_list_type const& ra , 
    address_list_type const& la = address_list_type()
  ) ;

  connection( std::string const& n , std::string const& s ) ;

  connection( acceptor& ) ;

  // Enable/disable nodelay
  void no_delay( bool = true ) ;

  // Set send/receive timeout, parameter must be >= 0.
  // Default: Wait indefinitely
  void    send_timeout( double ) ;
  void receive_timeout( double ) ;
  // Set both timeouts to the same value >= 0.
  // Default: Wait indefinitely
  void timeout        ( double ) ;

  address_type const& peer () const { return  peer_ ; }
  address_type const& local() const { return local_ ; }

  std::shared_ptr<cpl::detail_::stream_socket_reader_writer> socket() 
  { return s ; }

  cpl::detail_::socketfd_t fd() { return s->fd() ; }

  // buffer_maker<> interface
  instreambuf make_istreambuf() { return instreambuf( socket() ) ; }
  onstreambuf make_ostreambuf() { return onstreambuf( socket() ) ; }

private:

  void shutdown_read () { cpl::detail_::socket_shutdown_read ( fd() ); }
  void shutdown_write() { cpl::detail_::socket_shutdown_write( fd() ); }

  // *************** IMPORTANT ******************
  // This *must* be a shared pointer to share with the streambuf
  // classes.
  // Therefore we can:
  // * Have both instream and onstream on one connection
  // * Continue to use the streams after connection goes out of scope.
  std::shared_ptr< cpl::detail_::stream_socket_reader_writer > s ;

  // Tries to find a matching pair from remote address and local
  // address and returns an accordingly initialized socket.
  std::shared_ptr< cpl::detail_::stream_socket_reader_writer > initialize(
    address_list_type const& ra , 
    address_list_type const& la
  ) ;

  address_type local_ ;
  address_type  peer_ ;

} ;


struct acceptor {

  // Move, but don't copy
  acceptor(acceptor&&) = default;
  acceptor& operator=(acceptor&&) = default;

  acceptor(acceptor const&) = delete;
  acceptor& operator=(acceptor const&) = delete;

  typedef stream_address      address_type      ;
  typedef stream_address_list address_list_type ;

  // Listen on given local service, IPv4.
  acceptor( std::string       const& ls , int backlog = 0 ) ;

  // Listen on first suitable local address.
  acceptor( address_list_type const& la , int backlog = 0 ) ;

  address_type const& local() const { return local_ ; }

  cpl::detail_::socketfd_t fd() const { return s.fd() ; }

private:

  cpl::detail_::stream_socket_reader_writer s ;

  address_type local_ ;

} ;



//
// std::istream and std::ostream classes for stream (TCP) sockets.
//

typedef cpl::util::file::owning_istream<instreambuf> instream ;
typedef cpl::util::file::owning_ostream<onstreambuf> onstream ;

inline instream make_instream( connection& c )
{ return instream( c ) ; }

inline onstream make_onstream( connection& c )
{ return onstream( c ) ; }

#if 0
// Peer functions.
// TODO: These are dangerous because after shutdown getpeername()
// no longer works.  Cache the info in the onstream object or
// hold a reference to the connection.
stream_address peer( instream const& ) ;
stream_address peer( onstream const& ) ;
stream_address local( instream const& ) ;
stream_address local( onstream const& ) ;
#endif

#if 0
// Removed because we removed virtual inheritance from the streambuf
// classes.  We need to clearly understand virtual inheritance
// and its influence on constructors before this can be
// reintroduced.  
// Mixing input and output is often a bad idea anyway.
struct nstream : std::iostream {

  nstream( connection& c ) 
  : std::iostream( 0 ) ,
    buf          ( c )
  { rdbuf( &buf ) ; }

private:

  nstreambuf buf ;

} ;
#endif


} // namespace network

} // namespace util

} // namespace cpl


////////////////////////////////////////////////////////////////////////
// Template definitions.
////////////////////////////////////////////////////////////////////////

inline const char* notnull( const char* const s ) {
  return s ? s : "(null)";
}

template< int type >
std::vector< cpl::detail_::address< type > >
cpl::detail_::my_getaddrinfo(
  char const* const n ,
  char const* const s ,
  int const family_hint
) {

  // std::cout << "Resolving " << notnull(n) << ":" << notnull(s) << std::endl;

  always_assert( SOCK_DGRAM == type || SOCK_STREAM == type ) ;
  always_assert( n || s ) ;
  int const flags = n ? 0 : AI_PASSIVE ;

  addrinfo* res = 0 ;
  addrinfo  hints   ;

  // Make sure everything's cleared...
  std::memset(&hints, 0, sizeof(hints));

  hints.ai_flags      = flags       ;
  hints.ai_family     = family_hint ;
  hints.ai_socktype   = type        ;

  // Be extra clear, cf. http://en.wikipedia.org/wiki/Getaddrinfo :
  // In some implementations (like the Unix version for Mac OS), the 
  // hints->ai_protocol will override the hints->ai_socktype value while
  // in others it is the opposite, so both need to be defined with
  // equivalent values for the code to be cross platform.
  hints.ai_protocol   = SOCK_DGRAM == type ? IPPROTO_UDP : IPPROTO_TCP ;

  hints.ai_addrlen    = 0 ;
  hints.ai_addr       = 0 ;
  hints.ai_canonname  = 0 ;
  hints.ai_next       = 0 ;

  int const err = getaddrinfo( n , s , &hints , &res ) ;

  if( err ) {

    always_assert( !res ) ;
    
    if( n && s ) {

      throw std::runtime_error( 
          std::string( "can't resolve " )
        + n
        + ":" 
        + s
        + ": " 
        + ::gai_strerror( err ) 
      ) ;

    } else {
      
      throw std::runtime_error( 
          std::string( "can't resolve " )
        + s
        + ": " 
        + ::gai_strerror( err ) 
      ) ;

    }

  }

  std::vector< cpl::detail_::address< type > > ret ;

  for( addrinfo const* p = res ; p ; p = p->ai_next ) { 

    always_assert( type == p->ai_socktype ) ;
    ret.push_back( 
      address< type >( 
        *reinterpret_cast< sockaddr_storage* >( p->ai_addr    ) ,
                                                p->ai_addrlen
        )
      )
    ;

  }

  always_assert( res ) ;
  ::freeaddrinfo( res ) ;

  always_assert( ret.size() >= 1 ) ;

#if 0
  for (unsigned i = 0; i < ret.size(); ++i ) {
    std::cout << "Found: " << ret[i] << std::endl;
  }
#endif

  return ret ;

}


template< int type >
std::string const 
cpl::detail_::my_getnameinfo( 
  cpl::detail_::address< type > const& a ,
  bool             const node    ,
  bool             const numeric ,
  bool             const fqdn
) {

  char n[ NI_MAXHOST ] ;
  char s[ NI_MAXSERV ] ;

  int const flags = 
      ( numeric   ? NI_NUMERICHOST | NI_NUMERICSERV : NI_NAMEREQD )
    | ( fqdn      ? 0                               : NI_NOFQDN   )
    | ( a.dgram() ? NI_DGRAM                        : 0           )
  ;

  int const err =
    node ?
      getnameinfo
      ( a.sockaddr_pointer() , a.length() , n , NI_MAXHOST , 0 , 0 , flags )
    : 
      getnameinfo
      ( a.sockaddr_pointer() , a.length() , 0 , 0 , s , NI_MAXSERV , flags )
  ;

  if( err ) {

    const std::string t = node ? "node" : "service" ;
    throw std::runtime_error( 
        "can't get name associated with " + t + ": " + gai_strerror( err ) 
    ) ;

  }

  return node ? n : s ;

}


template< int type >
std::string const
cpl::detail_::address< type >::fqdn() const {

  check_family( family() ) ;

  return cpl::detail_::my_getnameinfo< type >( *this , true , false , true ) ;

}


template< int type >
std::string const
cpl::detail_::address< type >::host_name() const {

  check_family( family() ) ;

  return cpl::detail_::my_getnameinfo< type >( *this , true , false , false ) ;

}

template< int type >
std::string const
cpl::detail_::address< type >::port_name() const {

  check_family( family() ) ;

  return cpl::detail_::my_getnameinfo< type >( *this , false , false , false ) ;

}

template< int type >
std::string const
cpl::detail_::address< type >::host() const {

  check_family( family() ) ;

  return cpl::detail_::my_getnameinfo< type >( *this , true , true , false ) ;

}

template< int type >
std::string const
cpl::detail_::address< type >::port() const {

  return cpl::detail_::my_getnameinfo< type >( *this , false , true , false ) ;

}


template< typename for_it >
cpl::util::network::datagram_socket::size_type 
cpl::util::network::datagram_socket::receive( 
  for_it const& begin ,
  double const& t , 
  size_type const max
) {

  if( t >= 0 ) {

    // Watch fd to see if it has input

    fd_set rfds ;
    FD_ZERO(        &rfds ) ;
    FD_SET ( fd() , &rfds ) ;

    // Do not wait.

    ::timeval tv = cpl::detail_::to_timeval( t ) ;

    int const nfds = fd() + 1 ;     // Size of array containing sockets 
                                    // up to and including the one we set.

    int err ;
    do 
    { err = ::select( nfds , &rfds , 0 , 0 , &tv ) ; } 
    while( cpl::detail_::EINTR_repeat( err ) ) ;  
    // Some platforms including Linux update timeout.

    if( err < 0 ) { cpl::detail_::throw_socket_error( "select" ) ; }

    if( err == 0 ) { return timeout() ; }

  }

  std::vector< char > buffer( max ) ;

  long err ;
  do {

    source_.set_maxlength() ;

    err = 
      ::recvfrom(
         fd()           ,
         &buffer[ 0 ]   ,
         buffer.size()  ,
         0              ,
         source_.sockaddr_pointer() ,
         source_. socklen_pointer()
      ) ;

  } while( cpl::detail_::EINTR_repeat( err ) ) ;

  if( err < 0 )
  { cpl::detail_::throw_socket_error( "recvfrom" ) ; }

  assert( static_cast< size_type >( err ) <= buffer.size() ) ;

  std::copy( buffer.begin() , buffer.begin() + err , begin ) ;
  return err ;

}


// TODO: Factor out common code from send(begin, end) and 
// send(begin, end, dest).
template< typename for_it >
void cpl::util::network::datagram_socket::send(
  for_it const& begin ,
  for_it const& end
) {
 
  if( !connected() ) {
    throw std::runtime_error( "datagram send: disconntected socket" ) ;
  }

  std::vector< char > const buffer( begin , end ) ;

  long const result = 
      cpl::detail_::my_send( fd() , &buffer[ 0 ] , buffer.size() ) ;

  if( result < 0 ) {
    if( errno == ECONNREFUSED ) { return ; }
    cpl::detail_::throw_socket_error( "send" ) ;
  }

  always_assert( result == static_cast< long >( buffer.size() ) ) ;

}



template< typename for_it >
void cpl::util::network::datagram_socket::send(
  for_it const& begin ,
  for_it const& end   ,
  address_type const& d
) {

  // Can we send from an IPv4 socket to IPv6 or vice versa?  Probably 
  // this is system dependent, so don't rely on it.
  if( d.family() != local().family() ) {
    throw std::runtime_error( "datagram send: address family mismatch" ) ;
  }

  std::vector< char > const buffer( begin , end ) ;

  const long result = cpl::detail_::my_sendto(
      fd() , d , &buffer[ 0 ] , buffer.size() ) ;

  if( result < 0 ) {
    if( errno == ECONNREFUSED ) { return ; }
    cpl::detail_::throw_socket_error( "send to " + d.host() + ":" + d.port() ) ; 
  }

  always_assert( result == static_cast< long >( buffer.size() ) ) ;

}
 

template< typename for_it >
void cpl::util::network::datagram_socket::send( 
  for_it      const& begin   ,
  for_it      const& end     ,
  std::string const& node    , 
  std::string const& service
) {

  address_list_type const& ra = resolve_datagram( node , service ) ;

  for( auto const& i : ra ) {

    if( i.family() == local().family() ) {
      send( begin , end , i ) ; 
      return ;
    }

  }

  throw std::runtime_error( "datagram send: no matching address family" ) ;

}



template< int type >
long cpl::detail_::socket< type >::read
( char      * const buf , long const n ) {
  
  long ret ;
  do { ret = ::recv( fd() , buf , n , 0 ) ; }
  while( EINTR_repeat( ret ) ) ;

  assert( -1 <= ret      ) ;
  assert(       ret <= n ) ;

  return ret ;

}


template< int type >
long cpl::detail_::socket< type >::write
( char const* const buf , long const n ) {

  long ret ;

  // TODO: Handle partial send
  do { ret = cpl::detail_::socketsend( fd() , buf , n ) ; }
  while( EINTR_repeat( ret ) ) ;

  if( ret < 0 || n != ret )
  { return -1 ; }

  assert( n == ret ) ;
  return ret ;

}



#endif // CPP_LIB_NETWORK_H
