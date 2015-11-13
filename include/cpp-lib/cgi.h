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
// Component: CGI
//

#ifndef CPP_LIB_CGI_H
#define CPP_LIB_CGI_H

#include "cpp-lib/assert.h"

#include "boost/lexical_cast.hpp"

#include <map>
#include <string>
#include <utility>

namespace cpl {
namespace cgi {

// Transforms e.g. demo%3Amain into demo:main and returns the unescaped
// string.  If throw_on_errors is true, throws on malformed input,
// otherwise returns an incompletely decoded string. 
std::string uri_decode(
    std::string const& escaped, bool throw_on_errors = false);

// Parse an individual "key=value" pair
std::pair<std::string, std::string> parse_parameter(std::string const&);

// Parse a sequence of "key1=value1&key2=value2..."
std::map<std::string, std::string> parse_query(std::string const&);

// Given a map params, sets p to the value found in the map if present.
// Doesn't touch p otherwise.
template<typename T, typename M>
void set_value(M const& params, T& p, std::string const& name) {
  auto const it = params.find(name);
  if (params.end() != it) {
    p = boost::lexical_cast<T>(cpl::cgi::uri_decode(it->second));
  }
}

} // cpl
} // cgi


#endif // CPP_LIB_CGI_H
