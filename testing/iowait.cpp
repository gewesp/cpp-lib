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

////////////////////////////////////////////////////////////////////////
//
// ***
//
// THIS DOES NOT LINK.  THIS IS ONLY A TEST STUDY FOR A POSSIBLE
// SELECT()-LIKE INTERFACE.
//
// ***
//
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iterator>
#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <list>

#include <cassert>
#include <cctype>

#include "boost/shared_ptr.hpp"

#include "cpp-lib/sys/network.h"
#include "cpp-lib/util.h"


//
// Wait for incoming network activity on acceptors and streambufs.
//
// Preconditions:
//   *i is an acceptor for i in [acceptors_begin , acceptors_end)
//   *i is a streambuf for i in [streambufs_begin , streambufs_end)
//   
//   out_it_a dereferences to in_it_a
//   out_it_b dereferences to in_it_b
//
// Effects: 
//   Writes acceptors with connection request pending to acceptors_ready.
//   Writes streambufs with in_avail() > 0 to streambufs_ready.
//   If timeout (seconds) expires before any activity occurs, returns
//   without writing to the return iterators.  Negative timeout means
//   indefinite.
//

template< 
  typename in_it_a    , 
  typename in_it_b    , 
  typename out_cont_a , 
  typename out_cont_b 
> 
void iowait( 
  in_it_a const& acceptors_begin  ,
  in_it_a const& acceptors_end    ,
  in_it_b const& streambufs_begin ,
  in_it_b const& streambufs_end   ,
  out_cont_a  acceptors_ready       ,
  out_cont_b streambufs_ready_in    ,
  out_cont_b streambufs_ready_out   ,
  double const& timeout = -1
) ;


using namespace cpl::util::network ;


void usage( std::string const& name ) {

  std::cerr << 
"usage: " << name << " uppercase <port>\n"
"uppercase <port>:  Run uppercase server on <port>.\n"
  ;

}


//
// Pair of connections with their respective streambufs.
//

struct conn_sb {

  boost::shared_ptr< connection     > c  ;
  boost::shared_ptr< std::streambuf > sb ;

  std::streambuf& operator*() { return *sb ; }

} ;


typedef std::list< acceptor* > aclist ;
typedef std::list< conn_sb   > sblist ;


void run_uppercase_server( std::string const& port ) {
  
  aclist acs ;
  sblist sbs ;

  acceptor a( port ) ;
  acs.push_back( &a ) ;
  
  std::cerr << "Singlethreaded uppercase server version 0.01 listening on " 
            << a.local()
	    << std::endl ;

  while( 1 ) {

    std::vector< aclist::iterator > acs_ready     ;
    std::vector< sblist::iterator > sbs_ready_in  ;
    std::vector< sblist::iterator > sbs_ready_out ;
    
    iowait( 
      acs.begin() , acs.end() , 
      sbs.begin() , sbs.end() ,
      acs_ready     ,
      sbs_ready_in  ,
      sbs_ready_out
    ) ;


    // Handle new connections.

    for( 
      std::vector< aclist::iterator >::const_iterator i  = acs_ready.begin() ; 
                                                      i != acs_ready.end  () ; 
                                                    ++i 
    ) {

      conn_sb csb ;
      csb.c .reset( new connection( ***i   ) ) ; // doesn't block.
      csb.sb.reset( new nstreambuf( *csb.c ) ) ;

      sbs.push_back( csb ) ;
      std::cerr << "Connection from: " << csb.c->peer() << std::endl ;

    }
    
    // Handle existing connections.
    for( 
      std::vector< sblist::iterator >::const_iterator 
        i  = sbs_ready_in.begin() ; 
        i != sbs_ready_in.end  () ; 
      ++i 
    ) {

      std::streambuf&       sb = ***i ;
      std::streamsize const n  = sb.in_avail() ;

      if( 0 == n ) { 

        std::cerr << "Connection with " << ( *i )->c->peer() << " closed." 
                  << std::endl ;
        sbs.erase( *i ) ; 
        continue ; 

      }

      for( std::streamsize i = 0 ; i < n ; ++i ) {

        std::streambuf::int_type const c = sb.sgetc() ;
        assert( std::streambuf::traits_type::eof() != c ) ;
        sb.sputc( std::toupper( c ) ) ;

      }

    }

  }

}


int main( int argc , char const* const* const argv ) {

  try {

  if( argc != 3 ) {

    usage( argv[ 0 ] ) ; 
    return 1 ;

  }
  
  std::string const command = argv[ 1 ] ;

  if( std::string( "uppercase" ) == command ) {

    run_uppercase_server( argv[ 2 ] ) ;

  } else {
    
    usage( argv[ 0 ] ) ; 
    return 1 ;

  }

  } // end global try
  catch( std::exception const& e ) 
  { cpl::util::die( e.what() ) ; }

}
