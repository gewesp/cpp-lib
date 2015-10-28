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
// See reference [3] in README for a quick HTTP intro. 
//

#include "cpp-lib/http.h"

#include "cpp-lib/sys/network.h"
#include "cpp-lib/sys/syslogger.h"

using namespace cpl::util;
using namespace cpl::util::network;

namespace {

std::string const newline = "\r\n" ;


std::string const whitespace = "\t\n\r " ;

bool blank( std::string const& s ) {

  return std::string::npos == s.find_first_not_of( whitespace ) ;

}


void wget1( 
  std::ostream& log       ,
  std::ostream& os        ,
  std::string const& path , 
  double      const  timeout ,
  std::string const& host ,
  std::string const& port = "80" ,
  std::string const& from = "ano@nymous.com" ,
  std::string const& user_agent = "KISS/2015 httpclient/0.83"
) {
  
  connection c( host , port ) ;
  c.timeout( timeout ) ;

  std::ostringstream oss ;
  oss << "GET "         << path << " HTTP/1.0"  << newline
      << "From: "       << from                 << newline
      << "Host: "       << host << ":" << port  << newline
      << "User-Agent: " << user_agent           << newline
  // Connection: close would be HTTP/1.1
  //  << "Connection: close"                    << newline

      // Important: Don't miss the last (second!) newline
      << newline;
    
  log << cpl::util::log::prio::INFO 
      << "Requesting " << path << " from " << host << std::endl;
  
  onstream ons( c ) ;
  ons << oss.str() << std::flush ;
  
  instream ins( c ) ;
  std::string line ;

  while( std::getline( ins , line ) ) {

    if( blank( line ) ) { break ; }
    log << cpl::util::log::prio::INFO 
        << "Server HTTP header: " << line << std::endl ;
  
  }

  cpl::util::stream_copy( ins , os ) ;

}


} // end anonymous namespace

void cpl::http::wget( std::ostream& log, std::ostream& os , std::string url ,
                      double const timeout ) {

  if( "http://" != url.substr( 0 , 7 ) )
  { throw std::runtime_error( "URL must start with http://" ) ; }

  url = url.substr( 7 ) ;

  std::string::size_type const slash = url.find_first_of( '/' ) ;

  if( 0 == slash ) 
  { throw std::runtime_error( "bad URL format: No host[:port] parsed" ) ; }

  if( std::string::npos == slash )
  { throw std::runtime_error( "bad URL format: No slash after host[:port]" ) ; }
  
  std::string const path     = url.substr( slash     ) ;
  std::string const hostport = url.substr( 0 , slash ) ;

  std::string::size_type const colon = hostport.find_first_of( ':' ) ;

  if( hostport.size() - 1 == colon ) { 

    throw std::runtime_error
    ( "bad URL format: colon after hostname, but no port" ) ; 
  
  }
  
  if( 0 == colon ) { 

    throw std::runtime_error
    ( "bad URL format: no hostname before colon" ) ; 
  
  }

  std::string const port = std::string::npos == colon ? 
    "80" : hostport.substr( colon + 1 ) ;

  std::string const host = hostport.substr( 0 , colon ) ;

  assert( host.size() ) ;
  assert( port.size() ) ;

  wget1( log, os , path , timeout , host , port ) ;

}
