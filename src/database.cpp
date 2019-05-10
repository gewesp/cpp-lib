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

#include "cpp-lib/database.h"

#include <iostream>


void cpl::db::write(std::ostream& os, const cpl::db::table_statistics& stats) {
  os << "Memory consumption for table: " << stats.name << std::endl;
  os << "Table " << stats.name 
     << ": Number of items: " << stats.size << std::endl;
  os << "Table " << stats.name 
     << ": Estimated number of bytes: " << stats.bytes_estimate << std::endl;
  os << "Table " << stats.name 
     << ": Precise number of bytes: ";
  if (stats.bytes_precise >= 0) {
    os << stats.bytes_precise << std::endl;
  } else {
    os << "(not computed)" << std::endl;
  }
}
