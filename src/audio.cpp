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

#include "cpp-lib/audio.h"

#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <memory>

#include <cassert>

#include "cpp-lib/util.h"
#include "cpp-lib/xdr.h"

#include <boost/lexical_cast.hpp>

using namespace cpl::audio;
using namespace cpl::util;


cpl::audio::pcm_t 
cpl::audio::make_beep(
    double const amplitude,
    double const n,
    double const duration, 
    double const ontime,
    ramp_t const& ramp, 
    double const sample_rate) {
  
  verify(amplitude >= 0, "amplitude must be >= 0");
  verify(duration  > 0, "duration must be > 0");
  verify(0 <= ontime           , "ontime must be >= 0");
  verify(     ontime < duration, "ontime must be < duration");

  double const frequency = note(n);

  long const size = static_cast<long>(duration * sample_rate + .5);
  pcm_t ret(size);

  for (long i = 0; i < size; ++i) {
    double const t = i / sample_rate;
    ret[i] = amplitude * ramp(t, ontime) * std::sin(2 * M_PI * frequency * t);
  }
  return ret;
}

cpl::audio::pcm_t 
cpl::audio::make_beeps(
    double const amplitude,
    std::vector<std::vector<double> > const& params,
    ramp_t const& ramp,
    double sample_rate) {

  verify(3 == params.size(), "make_beeps: need 3 parameter vectors");

  const unsigned long n = params[0].size();
  verify(params[1].size() == n, 
         "make_beeps: parameter vectors must have same size");
  verify(params[2].size() == n, 
         "make_beeps: parameter vectors must have same size");

  pcm_t ret;
  for (unsigned long i = 0; i < n; ++i) {
    auto const beep = make_beep(
        amplitude, 
        params[0][i], params[1][i], params[2][i], ramp, sample_rate);
    ret.insert(ret.end(), beep.begin(), beep.end());
  }

  return ret;
}



void cpl::audio::write(
    std::string const& filename,
    pcm_t const& v,
    double const sample_rate) {
  auto os = cpl::util::file::open_write(filename);

  std::ostreambuf_iterator<char> it(os.rdbuf());

  std::uint32_t const magic = 0x2e736e64;
  std::uint32_t const data_offset = 24;
  std::uint32_t const data_size = 2 * v.size();  // data size, bytes
  // std::uint32_t const encoding = 6;  // 32-bit IEEE float
  std::uint32_t const encoding = 3;  // 16-bit PCM, signed
  std::uint32_t const rate = sample_rate + 0.5;
  std::uint32_t const channels = 1; // mono

  cpl::xdr::write(it, magic);
  cpl::xdr::write(it, data_offset);
  cpl::xdr::write(it, data_size);
  cpl::xdr::write(it, encoding);
  cpl::xdr::write(it, rate );
  cpl::xdr::write(it, channels);

  for (auto x : v) {
    std::int16_t const s = x * 32760 + .5;
    cpl::xdr::write(it, s);
  }
}
