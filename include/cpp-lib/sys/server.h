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
// Component: SYSUTIL
//
// ASCII line-based TCP server framework, suitable e.g. for telnet,
// HTTP etc.
//
// TODO:
// * Server shutdown (needs a way to interrupt the acceptor---timeout?)
//

#ifndef CPP_LIB_SYS_SERVER_H
#define CPP_LIB_SYS_SERVER_H

#include "cpp-lib/sys/network.h"

#include <boost/optional.hpp>

#include <functional>
#include <iosfwd>


namespace cpl {

namespace util {


//
// A server is built around a handler function.
//
// Handler parameters are:
// * The input line
// * Input and output streams bound to the network connection
// * A stream for logging.  A new stream is created for 
//   for each incoming connection.
//
// Handlers *must* be copyable.  Each thread serving a connection gets
// a copy of the handler.
//
// Handler return value:
//   The function must return false if the connection should be closed,
//   e.g. if the peer has issued a quit command.
//

typedef std::function<
  bool(std::string const& line,
       std::istream& ins,
       std::ostream& ons,
       std::ostream& log)> input_handler_type;


//
// This function is called at the beginning of a server to write a
// welcome message.
//

typedef std::function<
  void(std::ostream& ons)> os_writer;

//
// Connection parameters.
// bind_address    ... The local address to bind to, default: "0.0.0.0"
// service         ... Port, or "test:stdio" for test run on stdin/stdout
// server_name     ... Server name for syslog
// n_listen_retries ... Retry this many times to listen to incoming
//                      connections, typically to wait for the port to
//                      become available
// listen_retry_time ... Time to wait for listening port to become available
//                     after a failure to listn [s]
// log_connections ... Whether to log connections or not
//                     ("New connection", "Connection closing" log entries)
// max_line_length ... Maximum for input lines
// timeout         ... Connection timeout connection after this amount of 
//                     inactivity [s] (both input and output)
// backlog         ... Max number of backlogged connections (see acceptor)
// background      ... Whether to listen for connections in background or not
//
// For test mode, timeout, backlog and background are ignored.
//

struct server_parameters {
  server_parameters() {} 

  std::string bind_address = cpl::util::network::any_ipv4();
  std::string service    = "test:stdio";
  std::string server_name = "cpp-lib/generic";
  bool   log_connections = true ;
  long   n_listen_retries = 0   ;
  double listen_retry_time = 1.0 ;
  long   max_line_length = 1000 ;
  double timeout         = 60.0 ;
  int    backlog         = 0    ;
  bool   background      = false;
};


//
// Starts an IPv4 server on port params.service with the given
// backlog.
//
// Logs start with params.server_name in syslog or on the given
// ostream (sl), if non-null.
//
// If welcome is given, uses the function to write a message to its
// stream argument at the beginning of each connection.
//
// If params.background is true, starts the server in a separate thread
// and returns immediately.
// Otherwise, the constructor never returns.
//
// Each connection runs a getline() loop calling handler until it 
// returns false.
//
// THROWS: On errors listening to the port, e.g. privilege error
// or address already in use.  Exceptions in connection handlers
// are logged and the connection is terminated.
//
// CAUTION: getline() trims \n, but *not* \r, so there may be
// remaining whitespace at the end of the line.
//
// NOTE: You'll typically need to cast the welcome function to
// the os_writer type, i.e.
//   void welcome_func(std::ostream& os) { ... }
//   cpl::util::run_server( ..., cpl::util::os_writer{welcome_func}, ...);
//

void run_server(
    input_handler_type const& handler,
    boost::optional<os_writer> welcome = boost::none,
    server_parameters const& params = server_parameters(),
    std::ostream* sl = nullptr);


} // namespace util

} // namespace cpl


#endif // CPP_LIB_UTIL_SYS_H
