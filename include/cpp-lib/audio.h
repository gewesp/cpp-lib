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
// Component: AUDIO
//
// Audio synthesis, Sun .au/.snd output
//


#ifndef CPP_LIB_AUDIO_H
#define CPP_LIB_AUDIO_H

#include <vector>

#include <cassert>
#include <cmath>
#include <cstdint>

#include "cpp-lib/assert.h"

namespace cpl   {

namespace audio {

// Value representing one sample
typedef float sample_t;

// A sequence of samples
typedef std::vector<sample_t> pcm_t;

//
// Returns frequency of the note the given halftone above or
// below c''
//

inline double note(double const halftone)
{ return 523.25 * std::pow(2, halftone / 12); }

// Default sample rate [1/s]
inline double default_sample_rate() { return 48000; }


struct ramp_t {
  double  on_ramp ;
  double off_ramp;

  ramp_t(double const on_ramp, double const off_ramp)
    :  on_ramp( on_ramp),
      off_ramp(off_ramp) {
    check();
  }

  ramp_t(std::vector<double> const& params) {
    cpl::util::verify(2 == params.size(), "ramp: need 2 parameters");
    on_ramp = params[0];
    off_ramp = params[1];
    check();
  }

  void check() const {
    cpl::util::verify( on_ramp >= 0, "ramp: on parameter should be >= 0");
    cpl::util::verify(off_ramp >= 0, "ramp: off parameter should be >= 0");
  }

  double operator()(double t, double ontime) const {
    if (t < 0.0) {
      return 0.0;
    }
    if (t >= ontime) {
      return 0.0;
    } else {
      if (t < on_ramp) {
        return t / on_ramp;
      } else if (ontime - off_ramp < t) {
        return (ontime - t) / off_ramp;
      } else {
        return 1.0;
      }
    }
  }
};

pcm_t make_beep(
    double amplitude,
    double note, 
    double duration, 
    double ontime,
    ramp_t const& ramp, 
    double sample_rate = default_sample_rate());

// Takes a 3 x n-matrix of params: {
// { {note     ...},
//   {duration ...},
//   {ontime   ...}}
pcm_t make_beeps(
    double amplitude,
    std::vector<std::vector<double> > const& params, 
    ramp_t const& ramp, 
    double sample_rate = default_sample_rate());

//
// Write in .snd format using 16bit PCM signed encoding:
// http://en.wikipedia.org/wiki/Au_file_format
//

void write(std::string const& filename, 
           pcm_t const&, 
           double sample_rate = default_sample_rate());



} // namespace audio

} // namespace cpl


#endif 
