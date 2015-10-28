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

#include "cpp-lib/sys/file.h"
#include "cpp-lib/sys/util.h"
#include "cpp-lib/command_line.h"
#include "cpp-lib/serial.h"
#include "cpp-lib/util.h"

#include <boost/lexical_cast.hpp>

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <cassert>




using namespace cpl::util ;

namespace {

const opm_entry options[] = {

  opm_entry( "read"       , opp( false , 'r' ) ) ,
  opm_entry( "write"      , opp( false , 'w' ) ) ,
  opm_entry( "port"       , opp( true  , 'p' ) ) ,
  opm_entry( "delay"      , opp( true  , 'd' ) ) ,
  opm_entry( "config"     , opp( true  , 'c' ) ) ,
  opm_entry( "help"       , opp( false , 'h' ) )

} ;



void usage( std::string const& self) {
  std::cerr << self 
            << "       --read|--write                             \\\n"
            << "       [ --delay delay_between_lines_in_seconds ] \\\n"
            << "       [ --port COMx ]                            \\\n"
            << "       [ --config configuration ]                 \\\n"
            << "port is COM1, COM2, ...\n"
            << "Configuration is e.g. \"baud=19200 parity=N data=8 stop=1\"\n"
            << "Default: Read from stdin and write to port.\n"
  ;
}


void read(std::string const& port, std::string const& config) {
  cpl::serial::tty t( port , config ) ;

  std::cout << "Reading data from " << port << std::endl;
  std::cout << "Configuration: " << config << std::endl;

  std::string s ;
  while( std::getline( t.in , s ) ) { std::cout << s << std::endl ; }
}

void write(std::string const& port, 
           std::string const& config, 
           double const& delay) {
  cpl::serial::tty t( port , config ) ;

  std::cout << "Writing data to " << port << std::endl;
  std::cout << "Configuration: " << config << std::endl;

  std::string s ;

  sleep_scheduler ss(delay);

  while( std::getline( std::cin , s ) ) { 
    std::cout << ss.wait_next() << ": sending line: " << s << std::endl;
    t.out << s << "\r\n" << std::flush ;
  }
}

} // end anonymous namespace


int main( int const argc , char const* const* const argv ) {

  const command_line cl(options, options + size(options), argv);

  if (cl.is_set("help")) { usage(argv[0]); return 0; }
  if (cl.is_set("read") && cl.is_set("write")) { usage(argv[0]); return 0; }

  const std::string config = cl.get_arg_default(
      "config", 
      "baud=19200 parity=N data=8 stop=1");

  const std::string port = cl.get_arg_default("port", "COM9");

  const double delay = boost::lexical_cast<double>(
      cl.get_arg_default("delay", "1"));

  try {

  if (cl.is_set("read")) {
    read(port, config);
  } else {
    write(port, config, delay);
  }

  } catch( std::runtime_error const& e ) 
  { std::cerr << e.what() << std::endl ; return 1 ; }

}
