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

#include "cpp-lib/cgi.h"

#include "cpp-lib/error.h"
#include "cpp-lib/util.h"

#include "boost/algorithm/string.hpp"

#include <vector>

typedef boost::iterator_range<std::string::iterator> stringpiece;

std::pair<std::string, std::string>
cpl::cgi::parse_parameter(std::string const& s) {
  std::vector<stringpiece> keyval;
  std::string ss = s;
  boost::algorithm::split(keyval, ss, boost::algorithm::is_any_of("="));
  if (keyval.size() != 2) {
    throw std::runtime_error("CGI parse key/value pair: "
        + s + ": syntax error");
  }

  std::string const key{begin(keyval[0]), end(keyval[0])};
  std::string const val{begin(keyval[1]), end(keyval[1])};

  return std::make_pair(cpl::cgi::uri_decode(key), 
                        cpl::cgi::uri_decode(val));
}

std::pair<std::string, std::string>
cpl::cgi::split_uri(std::string const& s) {
  std::vector<std::string> v;
  cpl::util::split(v, s, "?");
  if (v.size() >= 3) {
    util::throw_error(
          "split_uri(): Expected at most one question mark, got "
        + std::to_string(v.size()));
  }

  if (1 == v.size()) {
    return std::pair(v[0], std::string());
  } else {
    return std::pair(v[0], v[1]);
  }
}

std::map<std::string, std::string> 
cpl::cgi::parse_query(std::string const& s) {
  std::string ss = s;
  boost::trim(ss);

  std::vector<stringpiece> fields;
  std::map<std::string, std::string> ret;

  if (ss.empty()) {
    return ret;
  }

  boost::algorithm::split(fields, ss, boost::algorithm::is_any_of("&"));
  for (auto const& field : fields) {
    auto const keyval = cpl::cgi::parse_parameter(
        std::string{begin(field), end(field)});
    if (ret.end() != ret.find(keyval.first)) {
      throw std::runtime_error("CGI parse: duplicate key: " + keyval.first);
    } else {
      ret.insert(std::move(keyval));
    }
  }
  return ret;
}

std::map<std::string, std::string> 
cpl::cgi::parse_query(std::istream& is) {
  std::string keyvals;
  is >> keyvals;
  if (!is) {
    throw std::runtime_error("CGI parse: istream read error");
  } else {
    return parse_query(keyvals);
  }
}

std::string 
cpl::cgi::uri_decode(
        std::string const& escaped, bool const throw_on_errors) {
  char hex[3];
  hex[2] = 0;

  std::string ret;
  std::string::const_iterator it = escaped.begin();
  while (it != escaped.end()) {
    if ('+' == *it) {
      ret.push_back(' ');
      ++it;
      continue;
    }
    if ('%' != *it) {
      ret.push_back(*it);
      ++it;
      continue;
    }

    ++it;
    if (escaped.end() - it < 2) {
      if (throw_on_errors) {
        throw std::runtime_error("syntax error in URI: " + escaped);
      } else {
        return ret;
      }
    }

    hex[0] = *it++;
    hex[1] = *it++;

    char* endptr;
    auto const c = std::strtol(hex, &endptr, 16);
    if (endptr != hex + 2) {
      if (throw_on_errors) {
        throw std::runtime_error("bad hex code in URI: " + escaped);
      } else {
        return ret;
      }
    } else {
      assert(0 <= c  );
      assert(c <= 256);
      ret.push_back(static_cast<char>(c));
    }
  }

  return ret;
}
