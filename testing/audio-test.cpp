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

#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <memory>

#include <cassert>

#include "cpp-lib/audio.h"
#include "cpp-lib/command_line.h"
#include "cpp-lib/container-util.h"
#include "cpp-lib/registry.h"
#include "cpp-lib/util.h"
#include "cpp-lib/xdr.h"


using namespace cpl::audio;
using namespace cpl::util ;
using namespace cpl::xdr  ;


std::string const SUFFIX = ".melody";

double const DEFAULT_ON_RAMP  = 0.005;
double const DEFAULT_OFF_RAMP = 0.015;

const cpl::util::opm_entry options[] = {
  cpl::util::opm_entry("help"    , cpl::util::opp(false, 'h')),
};


void usage( std::string const& name ) {

  std::cerr << 
"Creates .snd file for beeps found in source files.\n"
"Usage: " << name << " [ file.melody ... ]\n"
  ;

}


int main( int , char const* const* const argv ) {

  try {

  cpl::util::command_line cl( options , options + size( options ) , argv ) ;

  if (cl.is_set("help")) {
    usage(argv[0]);
    return 0;
  }


  std::string file;
  while(cl >> file) {
    registry const reg(file);

    auto const out = file::basename(file, SUFFIX) + ".snd";
    double const amplitude = reg.get_default("amplitude", 1.0);
    double const on_ramp = reg.get_default ("on_ramp" , DEFAULT_ON_RAMP);
    double const off_ramp = reg.get_default("off_ramp", DEFAULT_OFF_RAMP);

    double const sample_rate = 
        reg.get_default("sample_rate", default_sample_rate());

    // -2: inner lists must have same length
    auto const params = reg.check_vector_vector_double("melody", 3, -2);

    ramp_t const ramp(on_ramp, off_ramp);
    pcm_t melody = make_beeps(amplitude, params, ramp, sample_rate);
    // TODO: Write
    std::cout << "Writing " << out << "..." << std::endl;
    write(out, melody);
  }

  } // end global try
  catch( std::exception const& e ) { 
    usage(argv[0]);
    cpl::util::die( e.what() ) ; 
  }

}
