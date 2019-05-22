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
#include "cpp-lib/error.h"
#include "cpp-lib/util.h"

#include "boost/algorithm/string.hpp"


using namespace cpl::util;
using namespace cpl::util::network;

namespace {

std::string const newline = "\r\n" ;


std::string const whitespace = "\t\n\r " ;

bool blank( std::string const& s ) {

  return std::string::npos == s.find_first_not_of( whitespace ) ;

}

void throw_get_parse_error(const std::string& what) {
  cpl::util::throw_error("CPL HTTP GET request parser: " + what);
}

std::string default_user_agent() {
  return "KISS CPL/0.9.1 httpclient/0.9.1 (EXPERIMENTAL)";
}


void wget1( 
  std::ostream& log       ,
  std::ostream& os        ,
  std::string const& path , 
  double      const  timeout ,
  std::string const& host ,
  std::string const& port = "80" ,
  std::string const& from = "ano@nymous.com" ,
  std::string const& user_agent = default_user_agent()
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

const char* const cpl::http::endl = "\r\n";

std::string cpl::http::default_server_identification() {
  return "KISS/CPL httpd/0.9.1 (Linux)";
}

void cpl::http::write_content_type(std::ostream& os, const std::string& ct) {
  os << "Content-Type: " << ct << cpl::http::endl;
}

void cpl::http::write_date(std::ostream& os, double now) {
  if (now < 0) {
    now = cpl::util::utc();
  }

  os << "Date: " << cpl::util::format_datetime(now)
     << cpl::http::endl;
}

void cpl::http::write_connection(std::ostream& os, const std::string& what) {
  os << "Connection: " << what << cpl::http::endl;
}

void cpl::http::write_server(std::ostream& os, const std::string& server) {
  os << "Server: " << server << cpl::http::endl;
}

void cpl::http::write_http_header_200(
    std::ostream& os,
    const std::string& ct,
    double now,
    const std::string& server_id) {
  os << "HTTP/1.1 200 OK" << cpl::http::endl;
  cpl::http::write_date        (os, now      );
  cpl::http::write_server      (os, server_id);
  cpl::http::write_connection  (os, "close"  );
  cpl::http::write_content_type(os, ct       );

  os << cpl::http::endl;
}

void cpl::http::write_http_header_404(
    std::ostream& os,
    double now,
    const std::string& server_id) {
  os << "HTTP/1.1 404 Not Found" << cpl::http::endl;
  cpl::http::write_date        (os, now      );
  cpl::http::write_server      (os, server_id);
  cpl::http::write_connection  (os, "close"  );

  os << cpl::http::endl;
}

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

cpl::http::get_request
cpl::http::parse_get_request(const std::string& first_line, std::istream& is) {
  cpl::http::get_request ret;
  std::string line = first_line;
  boost::trim(line);
  {
    std::istringstream iss(line);
    std::string get, ver;
    iss >> get >> ret.abs_path >> ver;
    if (not iss) {
      ::throw_get_parse_error("Malformed request: " + line);
    }

    if ("GET" != get) {
      ::throw_get_parse_error("Not a GET request: " + line);
    }

    // Parse HTTP/x.y
    std::vector<std::string> ver1;
    cpl::util::split(ver1, ver, "/");

    if (2 != ver1.size() or "HTTP" != ver1.at(0)) {
      ::throw_get_parse_error("Bad version: " + ver);
    }

    ret.version = ver1.at(1);
  }

  while (std::getline(is, line)) {
    boost::trim(line);
    if (line.empty()) {
      break;
    }

    const auto hh = cpl::util::split_colon_blank(line);

           if ("User-Agent" == hh.first) {
      ret.user_agent = hh.second;
    } else if ("Host"       == hh.first) {
      ret.host       = hh.second;
    } else if ("Accept"     == hh.first) {
      ret.accept     = hh.second;
    } else {
      ::throw_get_parse_error("Unknown header field: " + hh.first);
    }
  }

  return ret;
}
