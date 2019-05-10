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
// Component: DB
//

#ifndef CPP_LIB_DATABASE_H
#define CPP_LIB_DATABASE_H

#include <iosfwd>
#include <string>

namespace cpl {

namespace db {

///
/// A struct representing static database statistics
///

struct table_statistics {
  /// Table name
  std::string name;

  /// Number of items stored
  double size = 0;

  /// Estimated number of bytes used
  double bytes_estimate = 0;

  /// Precise number of bytes used (may be slow to compute)
  /// A value of -1 causes output to be suppressed
  double bytes_precise = -1;
};

/// Writes statistics to ostream
void write(std::ostream&, const table_statistics&);

} // db

} // cpl

#endif // CPP_LIB_DATABASE_H
