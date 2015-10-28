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

#include "cpp-lib/command_line.h"
#include "cpp-lib/gnss.h"
#include "cpp-lib/igc.h"
#include "cpp-lib/util.h"


#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <cassert>


using namespace cpl::util ;

namespace {

const opm_entry options[] = {

  opm_entry( "convert" , opp( false , 'c' ) ),
  opm_entry( "rfdebug" , opp( false , 'r' ) ),
  opm_entry( "help"    , opp( false , 'h' ) )

} ;



void usage( std::string const& self) {
  std::cerr << self                       <<    "\n"
            << "       --convert [ --rfdebug ] files...\n"
  ;
}


void convert(std::istream& is, std::ostream& os, 
    const bool fixes, const bool rfdebug) {
  std::string line;
  cpl::gnss::fix f;

  while (std::getline(is, line)) {
    if (0 == line.size()) {
      std::cerr << "Warning: empty line!" << std::endl;
    }
    if ('B' == line[0]) {
      f = cpl::igc::parse_b_record(line);
      if (fixes) {
        os << "fix " << f << std::endl;
      }
    }

    if (rfdebug && 0 == line.find("LFLA") && line.size() > 20 &&
        '2' == line[10] && '9' == line[11]) {
      // RF debug entry
      char rf[2] = "";
      double cd = 0, am = 0, tx = 0, rx = 0, drop = 0;
      if (5 != std::sscanf(line.c_str(),
            "%*s RF %1[AB] CD %lf AM %lf TX %lf RX %lf %*s DROP %lf",
            rf, &cd, &am, &tx, &rx, &drop)) {
        throw std::runtime_error("Couldn't parse RF debug line");
      }

      if (f.n_satellites > 0) {
        os << "rfdebug " << f << " "
           << rf << " "
           << cd << " "
           << am << " "
           << rx << " "
           << drop << '\n'
        ;
      }
    }
  }
}

} // end anonymous namespace


int main( int const argc , char const* const* const argv ) {

  try {

  command_line cl(options, options + size(options), argv);

  if (cl.is_set("help")) { usage(argv[0]); return 0; }

  const bool fixes   = cl.is_set("convert");
  const bool rfdebug = cl.is_set("rfdebug");
  if (fixes || rfdebug) {
    std::string filename;

    while (cl >> filename) {
      std::ifstream is(filename.c_str());
      if (!is) {
        throw std::runtime_error("Couldn't read " + filename);
      }

      convert(is, std::cout, fixes, rfdebug);
    }
  } else {
    usage(argv[0]);
    return 1;
  }

  } catch( std::exception const& e ) 
  { std::cerr << e.what() << std::endl ; return 1 ; }

}
