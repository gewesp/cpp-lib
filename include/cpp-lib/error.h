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
// Component: UTIL
//
// Error handling.  Future versions may extend the exeptions to
// include things like error codes.  It may also define its own
// exception types.
//

#ifndef CPP_LIB_ERROR_H
#define CPP_LIB_ERROR_H

namespace cpl {

namespace util {

/// @throw A std::execption with the given message
void throw_error(const char* message);

/// @throw A std::exception with "Parse error: " + message
void throw_parse_error(const char* message);

} // namespace util

} // namespace cpl

#endif // CPP_LIB_ERROR_H
