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

#include "cpp-lib/sys/network.h"

#include <iostream>

namespace cpl {

namespace http {

inline std::string json_header() {
  return "Content-Type: application/json\n\n";
}

// Default timeout [s]
inline double default_timeout() { return 60; }

//
// Gets the specified URL and pipes the result to os.  Logs to log.
//

void wget( std::ostream& log , std::ostream& os , std::string url , 
           double timeout = default_timeout() ) ;


} // namespace http

} // namespace cpl


#endif // CPP_LIB_HTTP_H
