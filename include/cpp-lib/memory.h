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

#pragma once

#include "cpp-lib/assert.h"

#include <string>

#include <type_traits>

namespace cpl::detail_ {

long memory_consumption_overloaded(const std::string&);

} // namespace cpl::detail_

namespace cpl::util {

/// @return Estimate of memory used by the given object
template <typename T>
long memory_consumption(const T& x) {
  if constexpr(std::is_trivially_copyable<T>::value) {
    return sizeof(T);
  } else if constexpr(std::is_same<typename std::remove_cv<T>::type, std::string>::value) {
    return ::cpl::detail_::memory_consumption_overloaded(x);
  } else {
    // Doesn't work ... Did the committee not consider static_assert
    // in conjunction with constexpr if()?
    // static_assert(false, "memory_consumption() called for unsupported type");
    always_assert(not "memory_consumption() called for unsupported type");
  }
}

/// @return Estimate of memory consumption for containers (slow)
template <typename C>
long memory_consumption_container(const C& c) {
  long ret = 0;
  for (const auto& el : c) {
    ret += memory_consumption(el);
  }
  return ret;
}

/// @return Estimate of memory consumption for a std::map (slow)
template <typename M>
long memory_consumption_map(const M& m) {
  long ret = 0;
  for (const auto& el : m) {
    ret += memory_consumption(el.first )
        +  memory_consumption(el.second);
  }
  return ret;
}

} // namespace cpl::util
