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

#ifndef CPP_LIB_LOGGER_H
#define CPP_LIB_LOGGER_H

#include <any>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include "cpp-lib/util.h"
#include "cpp-lib/registry.h"
#include "cpp-lib/sys/network.h"

namespace cpl {

namespace util     {


//
// A Logger class primarily targeted at logging numerical values in
// real-time environments.
//

struct Logger {

  //
  // Initialize with empty list of known variables and log list, and
  // the given source port.
  //
  // TODO: Support IPv6 etc.
  Logger( std::string const& source_port ) 
  : s( new cpl::util::network::datagram_socket( cpl::util::network::ipv4 , 
                                                source_port ) ) , 
    ss( 0 ) {}

  //
  // Map variable name to variable.  variable must be a const pointer
  // to a supported type.  Currently, supported types are:
  //
  //   float
  //   double
  //   int
  //
  // The address of the variable must remain valid as long as log() is
  // called.
  //
  // Variables may be re-bound (e.g. after a reconfiguration causing an
  // address change).
  //
  // Each bind() call clears the log list.
  //
  
  template< typename T >
  void template_bind( std::string const& name , T const* const variable ) {

	if( !checkType( variable ) )
	{ throw std::runtime_error
	  ( "Logger::bind: " + name + ": unsupported data type" ) ; }

	known[ name ] = variable ;

	clear() ;

  }

  void bind( std::string const& name , float  const* const variable )
  { template_bind( name , variable ) ; }
  
  void bind( std::string const& name , double const* const variable )
  { template_bind( name , variable ) ; }
  
  void bind( std::string const& name , int    const* const variable )
  { template_bind( name , variable ) ; }

  //
  // Maps the names name + `i' to variable[ i ] for all i in the vector.
  // No size changing operations on vector are allowed afterwards.
  //

  void bind
  ( std::string const& name , std::vector< double > const* const variable ) ;

  //
  // Send subsequent packets to the given destination.
  //

  void logTo( std::string const& node , std::string const& service ) ;


  //
  // Send subsequent packets from the given source port.
  //

  void logFrom( std::string const& source_port ) ;

  //
  // Log the variables listed in names.  names may only contain variable
  // names for which bind() was called.
  //
  // If names contains an entry for which IsKnown() is false, the
  // list of logged variables is unchanged.
  //

  void setList( std::vector< std::string > const& names ) ;


  //
  // Clear list of variables to log.
  //

  void clear() { to_log.clear() ; }

  //
  // Set time interval to use by log( t ).  If dt is zero, every call to
  // log( t ) will send a packet.  dt must be nonnegative.
  //

  void setInterval( double const& dt ) ;
  
  //
  // Send a logging packet.  It contains the variables given in the last
  // setList() call in this order.  Values are printed in
  // whitespace-separated ASCII in standard C scientific format with
  // precision as high as sensible for the respective type.
  //
  // A packet is only sent if dt seconds have passed since last log()
  // call.  Otherwise, nothing happens.
  //
  // dt is the interval given in setInterval().  t is the current time.
  // The Logger class does not internally measure time by any other
  // means.
  //

  void log( double const& t ) ;


  //
  // true iff name was given in a successful bind() call.
  //

  bool IsKnown( std::string const& name ) const
  { return known.count( name ) > 0 ; }

  //
  // Check if a contains a const pointer to a supported type.
  // Currently, supported types are:
  //
  //   float
  //   double
  //   int
  //

  static bool checkType( std::any const& a ) ;

  
  //
  // The numerical precison to use (number of decimal digits).
  //

  enum { Precision = 16 } ;

  //
  // Whitespace character to use between values in logging packet.
  //

  static char const Whitespace = ' ' ;

private:

  typedef std::map< std::string , std::any > known_map ;

  std::unique_ptr< cpl::util::network::datagram_socket > s ;
  cpl::util::network::datagram_socket::address_type destination ;
  std::map< std::string , std::any > known ;
  std::vector< known_map::const_iterator > to_log ;

  cpl::util::simple_scheduler ss ;

  // noncopyable
  Logger const& operator=( Logger const& ) ;
  Logger                 ( Logger const& ) ;

  // caching char vector.
  std::vector< char > v ;

} ;


//
// Configure a logger using entries from a registry.  Entries (and 
// entry types) used are:
//
//   basename + "udp_host"    :  Hostname or IP address of destination
//                               host (string or identifier).
//
//   basename + "udp_port"    :  Destination port (string; numeric or 
//                               service name).
//
//   basename + "source_port" :  Source port (string; numeric or service
//                               name). May be undefined in which case a
//                               different port is used on each log() call.
//
//   basename + "variables"   :  Names of variables to log (list of
//                               strings or identifiers).
//
//   basename + "dt"          :  Logging interval.
//   

void configure
( Logger& , registry const& , std::string const& basename = "" ) ;


} // namespace cpl

} // namespace util

#endif // CPP_LIB_LOGGER_H
