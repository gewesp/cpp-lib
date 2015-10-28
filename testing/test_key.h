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

#ifndef CPP_LIB_TEST_KEY_H
#define CPP_LIB_TEST_KEY_H

#include <vector>
#include <cassert>

#include "cpp-lib/util.h"

inline std::vector< char > const key() {

  unsigned char const k[] = { 
	0x70 , 0x63 , 0xa7 , 0x56 , 0x05 , 0xd1 , 0xa8 , 0x26 , 
	0x5b , 0xc3 , 0xe8 , 0x8b , 0x15 , 0x6a , 0xf6 , 0x86 ,
	0x3a , 0xbe , 0x76 , 0xb6 , 0xd5 , 0xf6 , 0xcd , 0xb0 , 
	0x6c , 0x12 , 0x11 , 0x73 , 0x55 , 0x96 , 0xff , 0xb9
  } ;

  return std::vector< char >( k , k + cpl::util::size( k ) ) ;

}


inline std::vector< char > const IV() {

  unsigned char const i[] = { 
	0xb7 , 0xb3 , 0xeb , 0x5e , 0x75 , 0xb6 , 0x9a , 0x46 
  } ;

  assert( 8 == cpl::util::size( i ) ) ;
  
  return std::vector< char >( i , i + cpl::util::size( i ) ) ;

}

#endif // CPP_LIB_TEST_KEY_H
