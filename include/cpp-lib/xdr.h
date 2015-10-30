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
//
// XDR representation: See https://www.ietf.org/rfc/rfc1832.txt
// XDR is big-endian, that is, the most significant byte comes first.
//
// This header provides unoptimized read/write functions for signed/unsigned
// integers, floating point and fixed size strings.
//
// * All functions are templated on an iterator type that *must* point
//   to char.
// * The iterator is passed by reference and advanced to the next item.
// * Write is templatized on the value type:
//     cpl::xdr::write(it, value);
// * For read, the value type needs to be specified, e.g.:
//     cpl::xdr::read_int32 (it);
//     cpl::xdr::read_uint64(it);
//     cpl::xdr::read_double(it);
//
// We assume 2's complement representation for integers and IEEE for floats.
//
// TODO/WARNING:
// * 16 bit variants *do not pad* to multiples of 4 bytes!!
// * Remove 16 bit variants?  XDR is always on multiples of 4...
//

#ifndef CPP_LIB_XDR_H
#define CPP_LIB_XDR_H

#include "cpp-lib/assert.h"
#include "cpp-lib/units.h"
#include "cpp-lib/sys/file.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <typeinfo>
#include <type_traits>


namespace cpl {

namespace xdr {

template<bool is_signed, int bits> struct integer {};

template<> struct integer<true , 16> { typedef typename std::int16_t  type; };
template<> struct integer<true , 32> { typedef typename std::int32_t  type; };
template<> struct integer<true , 64> { typedef typename std::int64_t  type; };

template<> struct integer<false, 16> { typedef typename std::uint16_t type; };
template<> struct integer<false, 32> { typedef typename std::uint32_t type; };
template<> struct integer<false, 64> { typedef typename std::uint64_t type; };

// Returns next multiple of 4
template<typename T> unsigned padsize(T const n) {
  static_assert( std::is_integral<T>::value, "T must be integer type");
  static_assert(!std::is_signed  <T>::value, "T must be unsigned"    );

  T const ret = (0 == n % 4) ? 0 : 4 - (n % 4);
  always_assert(ret <= 3);
  return ret;
}

template<typename IT> void pad(IT& it, unsigned const padsize) {
  for (unsigned i = 0; i < padsize ; ++i) {
    *it = 0;
    ++it;
  }
}

template<typename IT> void skip(IT& it, unsigned const padsize) {
  for (unsigned i = 0; i < padsize; ++i) { ++it; }
}

template<typename IT>
void check_char(IT const it) {
  static_cast<void>(it);
#if 0
  // TODO---not clear why this fails
  static_assert(std::is_same<char, decltype(*it)>::value, 
                "iterator it should point to char");
#endif
  // This *also* doesn't work...
  // static_assert(1 == sizeof(*it), "iterator should point to char");
}

template<bool is_signed, int bits, typename IT> 
typename integer<is_signed, bits>::type read_integer(IT& it) {
  check_char(it);
  int constexpr size = bits / 8;

  typedef typename integer<false, bits>::type unsigned_type;
  static_assert(std::is_same<unsigned_type,
                typename std::make_unsigned<
                    typename integer<is_signed, bits>::type>::type>::value,
                "WTF");

  unsigned_type us = 0;
  for (int i = size - 1; i >= 0; --i) {
    us |= (static_cast<unsigned_type>(static_cast<unsigned char>(*it)) 
           << 8 * i);
    ++it;
  }
  return static_cast<typename integer<is_signed, bits>::type>(us);
}

template<typename T, typename IT> void write(IT& it, T const v) {
  static_assert(std::is_integral<T>::value,
      "T must be integer type");
  check_char(it);
  int constexpr size = sizeof(T);
  static_assert(2 == size || 4 == size || 8 == size,
      "T must be 16, 32 or 64 bit");

  typedef typename std::make_unsigned<T>::type unsigned_type; 
  static_assert(sizeof(T) == sizeof(unsigned_type), "WTF");
  unsigned_type const v_us = static_cast<unsigned_type>(v);

  for (int i = size - 1; i >= 0; --i) {
    *it = static_cast<char>((v_us >> 8 * i) & 0xff);
    ++it;
  }
}


template<typename IT, typename T> void write_raw(IT& it, T const& v) { 
  char const* const c = reinterpret_cast<char const*>(&v);
  for (unsigned i = 0; i < sizeof(T); ++i) {
    *it = c[i];
    ++it;
  }
}

template<typename T, typename IT> T read_raw(IT& it) {
  char buf[sizeof(T)];
  for(unsigned i = 0; i < sizeof(T); ++i) {
    buf[i] = *it;
    ++it;
  }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  return *reinterpret_cast<T const*>(&buf[0]);
#pragma GCC diagnostic pop 
}

template<typename IT> void write(IT& it, double const v) 
{ write_raw(it, v); }

template<typename IT> void write(IT& it, float const v) 
{ write_raw(it, v); }

template<typename IT> double read_double(IT& it)
{ return read_raw<double>(it); }

template<typename IT> double read_float(IT& it)
{ return read_raw<float>(it); }

template<typename IT> void write(IT& it, std::string const& s) {
  for (auto c : s) {
    *it = c;
    ++it;
  }
  pad(it, padsize(s.size()));
}

template<typename IT>
std::string read_string(IT& it, unsigned long const size) {
  std::string ret;
  for (unsigned long i = 0; i < size; ++i) {
    ret.push_back(*it);
    ++it;
  }
  skip(it, padsize(size));
  return ret;
}

#if 0
// Signed integers
template<typename IT> int16_t read_s16(IT& it) 
{ return read<int16_t, IT, 2>(it); }

template<typename IT> int16_t read_s32(IT& it) 
{ return read<int32_t, IT, 4>(it); }

template<typename IT> int16_t read_s64(IT& it) 
{ return read<int64_t, IT, 8>(it); }

// Unsigned integers
template<typename IT> uint16_t read_u16(IT& it) 
{ return read<uint16_t, IT, 2>(it); }

template<typename IT> uint16_t read_u32(IT& it) 
{ return read<uint32_t, IT, 4>(it); }

template<typename IT> uint16_t read_u64(IT& it) 
{ return read<uint64_t, IT, 8>(it); }
#endif

} // namespace xdr

} // namespace cpl

#endif // CPP_LIB_XDR_H
