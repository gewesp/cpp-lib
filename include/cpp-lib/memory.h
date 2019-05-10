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
// Experimental, unstable API
//
// Estimate memory consumption of PODs, standard containers etc.
//
// Component: UTIL
//

#ifndef CPP_LIB_MEMORY_H
#define CPP_LIB_MEMORY_H

#include "cpp-lib/assert.h"

#include <string>

#include <type_traits>

namespace cpl {

namespace detail_ {

long memory_consumption_overloaded(const std::string&);

} // namespace detail_

namespace util {

// TODO: Add overloads for lots of containers...
// Use constexpr if (C++17).

/// @return Estimate of memory used by the given object
template <typename T>
long memory_consumption(const T& x) {
  if (std::is_trivially_copyable<T>::value) {
    return sizeof(T);
  } else {
    always_assert(not "memory_consumption() called for unsupported type");
  }
}

/// @return Estimate of memory used by the given string
template <> inline long memory_consumption<std::string>(const std::string& x) {
  return ::cpl::detail_::memory_consumption_overloaded(x);
}

} // util

} // cpl

#endif // CPP_LIB_MEMORY_H
