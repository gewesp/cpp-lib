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
#include <string>
#include <exception>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <cassert>
#include <cstdlib>

#include "boost/lexical_cast.hpp"

#include "cpp-lib/sys/network.h"
#include "cpp-lib/util.h"

// Timeout for ping replies [s]
long const TIMEOUT = 5;

using namespace cpl::util::network ;
using namespace cpl::util          ;


namespace {

void usage( std::string const prog ) {

  std::cout 
    << "usage: " << prog << "\n"
"resolve              <host> <port>: Resolve given <host>:<port> combination\n"
"send         <proto> <host> <port>: Send to destination, reading lines from\n" 
"                                    stdin\n"
"receive      <proto> <listen_port>: Receive on <listen_port>, write to\n" 
"                                    stdout\n"
"receive-file <proto> <listen_port>: Receive on <listen_port>, write packets\n"
"                                    to a file sequence\n"
"ping         <proto> <host> <port> [ <msg> ... ]:\n"
"                                    Connect to <host>:<port>, send\n"
"                                    messages, print replies\n"
"pong         <proto> <listen_port>: Listen on <listen_port> and reply to\n"
"                                    received messages\n"
"\n"
"The <proto> argument refers to protocol version for the local socket\n"
"and must be ip4 or ip6.\n"
"\n"
"Examples:\n"
"  udp-test pong ip6 4711\n"
"  In another window: udp-test ping ip6 ::1 4711 \"Hi there\" \"Hi again\"\n"
  ;

}

//
// Return a zero padded string representing floor( x ).
//

std::string const zero_padded_string
( double const& x , unsigned const d ) {

  std::ostringstream os ;

  os << std::fixed
     << std::setw( d )
     << std::right
     << std::setfill( '0' )
     << std::setprecision( 0 )
     << x
  ;

  return os.str() ;

}

void ping( 
    std::string const& proto ,
    std::string const& host, 
    std::string const& port , 
    const char* const* msgs ) {

  datagram_socket s( address_family( proto ) ) ;
  datagram_socket::address_type source ;
  s.connect( host , port ) ;

  std::cout << "Local address: " << s.local() << std::endl ;
  std::cout << "Peer address: "  << s.peer () << std::endl ;

  while ( *msgs ) {
    std::cout << "Sending: " << *msgs << std::endl ;
    s.send( *msgs, *msgs + std::strlen( *msgs ) ) ;

    std::string reply;
    if ( s.timeout() == s.receive(
          source , std::back_inserter( reply ) , TIMEOUT ) ) {
      std::cout << "Reply timed out" << std::endl ;
    } else {
      std::cout << "Reply from " << source << ": " << reply << std::endl ;
    }

    ++msgs;
  }

}

void pong(std::string const& proto ,
          std::string const& listen_port ) {
  
  datagram_socket s( address_family( proto ) , listen_port ) ;
  datagram_socket::address_type source ;

  std::cout << "Receiving on local address: " << s.local() << std::endl ;

  long n = 0;
  while ( true ) {
    std::string msg ;
    s.receive( source , std::back_inserter( msg ) ) ;
    std::cout << "Received from " << source << ": " << msg << std::endl ;
    std::string reply = "Re: " + msg 
      + " (#" + boost::lexical_cast<std::string>(n) + ")" ;
    std::cout << "Sending reply: " << reply << std::endl ;
    s.send( reply.begin() , reply.end() , source ) ;
    ++n;
  }

}

#if 0
// Check that a default-constructed sender doesn't have a local address.
void unbound_local() {
 
  datagram_socket s;
  expect_throws( s.local() , std::runtime_error , "unbound datagram socket" ) ;

}
#endif

void run_tests() {
  // unbound_local();
  {
    datagram_socket s( ipv4 ) ;
    expect_throws( datagram_socket s1( ipv4 ) , 
                   std::exception , 
                   "in use" ) ;
  }
}


} // anonymous namespace


int main( int argc , char const* const* const argv ) {

  try {
  
  run_tests();
  
  if( argc < 2 ) 
  { usage( argv[ 0 ] ) ; return 1 ; }

  if(    std::string( "receive"      ) == argv[ 1 ]
      || std::string( "receive-file" ) == argv[ 1 ] ) {

    if( argc != 4 )
    { usage( argv[ 0 ] ) ; return 1 ; }

    datagram_socket r( address_family( argv[ 2 ] ) , argv[ 3 ] ) ;
    datagram_socket::address_type source;

    std::cerr << "Receiving packets on: " << r.local() << std::endl ;

    long n = 0 ;
    bool const to_file = std::string( "receive-file" ) == argv[ 1 ] ;
    double time = 0;
    
    while( true ) {

      datagram_socket::size_type size ;

      if( !to_file ) {

        std::cerr << "Data: " << std::flush ;
        size = r.receive( 
            source , std::ostreambuf_iterator< char >( std::cout ) ) ;
        time = cpl::util::utc();
        std::cout << std::endl ;

      } else {

        std::ofstream os( 
          ( "udp-receive-" + zero_padded_string( n , 8 ) ).c_str() 
        ) ;

        always_assert( os ) ;

        size = r.receive( 
            source , std::ostreambuf_iterator< char >( os ) ) ;
        time = cpl::util::utc();

        always_assert( os ) ;

      }
      
      ++n ;
      
      std::cerr 
        << cpl::util::format_datetime(time)
        << ": "
        << "#" << n 
        << ", size: " 
        << size
        << " bytes, source: "
        << source
        << std::endl
      ;

    }

  } else if( std::string( "send" ) == argv[ 1 ] ) {

    // Send!

    if( 5 != argc )
    { usage( argv[ 0 ] ) ; return 1 ; }

    datagram_socket s( address_family( argv[ 2 ] ) ) ;
    s.connect( argv[ 3 ] , argv[ 4 ] ) ;
    
    std::cout << "Sending from " << s.local() << " " ;

    std::cout << "to " << s.peer() << std::endl ;
    std::cout << "Please enter lines of data, Ctrl-D to exit:" << std::endl ;

    std::string ss ;
    while( std::getline( std::cin , ss ) )
    { s.send( ss.begin() , ss.end() ) ; }

  } else if( std::string( "resolve" ) == argv[ 1 ] ) {

    if( argc != 4 ) 
    { usage( argv[ 0 ] ) ; return 1 ; }

    datagram_address_list const& dest = 
      resolve_datagram( argv[ 2 ] , argv[ 3 ] ) ;

    std::cout << argv[ 2 ] << ":" << argv[ 3 ] << " resolves to:"
              << std::endl ;
    if( dest.empty() ) {
      std::cout << "(none)" << std::endl ;
    } else {
      for( unsigned i = 0 ; i < dest.size() ; ++i ) {
        std::cout << dest[ i ] << std::endl ;
      } 
    }

  } else if( std::string( "ping" ) == argv[1] ) {
    
    if( argc < 5 )
    { usage( argv[ 0 ] ) ; return 1 ; }

    ping( argv[2], argv[3] , argv[ 4 ] , &argv[ 5 ] ) ;

  } else if( std::string( "pong" ) == argv[1] ) {

    if( 4 != argc )
    { usage( argv[ 0 ] ) ; return 1 ; }

    pong( argv[2], argv[3] ) ;

  } else {
    throw std::runtime_error( std::string( "Unknown command: " ) + argv[1] ) ;
  }

  } // end global try
  catch( std::exception const& e ) {

    std::cerr << e.what() << std::endl ;
    return 1 ;

  }

}
