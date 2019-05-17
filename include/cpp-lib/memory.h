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
#include "cpp-lib/type_traits.h"

#include <string>

#include <type_traits>

namespace cpl::util {

/// @return Estimate of memory used by the given object
template <typename T> long memory_consumption(const T&);

} // namespace cpl::util


////////////////////////////////////////////////////////////////////////
// Implementation details only below this line
////////////////////////////////////////////////////////////////////////

namespace cpl::detail_ {

long memory_consumption_overloaded(const std::string&);

/// @return Estimate of memory consumption for containers.  Fast iff
/// value is a POD.
template <typename C>
long memory_consumption_container(const C& c) {
  using ::cpl::util::memory_consumption;

  if constexpr(std::is_trivially_copyable<typename C::value_type>::value) {
    return sizeof(typename C::value_type) * c.size();
  } else {
    long ret = 0;
    for (const auto& el : c) {
      ret += memory_consumption(el);
    }
    return ret;
  }
}

template <typename P>
long memory_consumption_pair(const P& p) {
  using ::cpl::util::memory_consumption;
  return memory_consumption(p.first) + memory_consumption(p.second);
}

} // namespace cpl::detail_



////////////////////////////////////////////////////////////////////////
// Template definitions
////////////////////////////////////////////////////////////////////////

template <typename T> long ::cpl::util::memory_consumption(const T& x) {
  if constexpr(std::is_trivially_copyable<T>::value) {
    return sizeof(T);
  } else if constexpr(std::is_same<typename std::remove_cv<T>::type, std::string>::value) {
    return ::cpl::detail_::memory_consumption_overloaded(x);
  } else if constexpr(::cpl::util::is_container<T>::value) {
    return ::cpl::detail_::memory_consumption_container(x);
  } else if constexpr(::cpl::util::is_pair<T>::value) {
    return ::cpl::detail_::memory_consumption_pair(x);
  } else {
    // Doesn't work ... Did the committee not consider static_assert
    // in conjunction with constexpr if()?
    // static_assert(false, "memory_consumption() called for unsupported type");
    always_assert(not "memory_consumption() called for unsupported type");
  }
}
