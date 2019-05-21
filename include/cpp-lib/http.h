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
// Component: HTTP
//


#ifndef CPP_LIB_HTTP_H
#define CPP_LIB_HTTP_H

#include <iosfwd>
#include <string>

namespace cpl {

namespace http {

inline std::string json_header() {
  return "Content-Type: application/json\n\n";
}

/// Default timeout for HTTP connections [s]
inline double default_timeout() { return 60; }

/// Gets the specified URL and pipes the result to os.  Logs to log.
void wget( std::ostream& log , std::ostream& os , std::string url , 
           double timeout = default_timeout() ) ;


/// Data of an HTTP GET request
/// https://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html
struct get_request {
  /// The HTTP version (1.0, 1.1 etc.)
  std::string version;

  /// The host field of the URI
  std::string host;

  /// The absolute path of the URI
  std::string abs_path;

  /// Contents of the 'User-Agent:' field
  std::string user_agent;

  /// Contents of the 'Accept:' field
  std::string accept;
};

/// Parses a GET request from the given istream.  Parses up to the first empty
/// line.
get_request parse_get_request(std::istream&);

} // namespace http

} // namespace cpl


#endif // CPP_LIB_HTTP_H
