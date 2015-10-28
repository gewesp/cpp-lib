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


#ifndef CPP_LIB_TOP_N_H
#define CPP_LIB_TOP_N_H

#include <set>

#include <cassert>
#include <cstdlib>


namespace cpl {

namespace util {

//
// A data structure to keep the top N elements in sorted order.
//
// Memory usage: O( N )
//
// TODO: Use C++11 allocators for fast performance.
//

template< typename T , int N , typename C = std::less< T > > struct top_n {

  typedef std::multiset< T , C > map_type ;
  typedef typename map_type::      iterator       iterator ;
  typedef typename map_type::const_iterator const_iterator ;

  static_assert( N > 0 , "N must be positive" ) ;

  // Iterate over sorted range
  const_iterator begin() const { return map_.begin() ; }
  const_iterator end  () const { return map_.end  () ; }

  int capacity() const { return N ; }

  int size() const { return map_.size() ; }

  bool empty() const { return map_.empty() ; }

  void clear() { map_.clear() ; }

  T const& front() const { return *map_.begin() ; }

  void push( T const& t ) {
    map_.insert( t ) ;
    if ( map_.size() > 
         static_cast< typename map_type::size_type >( capacity() ) ) {
      map_.erase( --map_.end() ) ;
      assert( size() <= capacity() ) ;
    }
  }

  void push( T&& t ) { push( t ) ; }

private:
  map_type map_ ;

} ;

} // namespace util

} // namespace cpl

#endif // CPP_LIB_TOP_N_H
