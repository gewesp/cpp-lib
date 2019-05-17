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
// Type traits that are missing in the standard library
//
// WARNING: Pulls in lots of headers.
//
// Component: UTIL
//

#pragma once

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <type_traits>

namespace cpl::detail_ {

template <typename T> struct is_container : std::false_type {};
template <typename T> struct is_pair      : std::false_type {};

template <typename T, std::size_t N> struct is_container<std::array    <T,N>>     : std::true_type {};
template <typename... ARGS> struct is_container<std::vector            <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::deque             <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::list              <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::forward_list      <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::set               <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::multiset          <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::map               <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::multimap          <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::unordered_set     <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::unordered_multiset<ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::unordered_map     <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::unordered_multimap<ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::stack             <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::queue             <ARGS...>> : std::true_type {};
template <typename... ARGS> struct is_container<std::priority_queue    <ARGS...>> : std::true_type {};

template <typename... ARGS> struct is_pair<std::pair<ARGS...>> : std::true_type {};

} // namespace cpl::detail_

namespace cpl::util {

/// is_container<T>::value is true iff T is one of the standard containers
template <typename T> struct is_container {
  static constexpr bool const value = ::cpl::detail_::is_container<std::decay_t<T>>::value;
};

/// is_pair<T>::value is true iff T is a std::pair<>
template <typename T> struct is_pair {
  static constexpr bool const value = ::cpl::detail_::is_pair<std::decay_t<T>>::value;
};

} // namespace cpl::util
