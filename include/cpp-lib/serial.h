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
// Component: UNSUPPORTED
//
// Windows only implementation of serial (COM) port streams with support
// for configuration of Baud rate etc.
//
// Unsupported: Virtual COM ports like the ones provided by com0com.
//

#ifndef CPP_LIB_SERIAL_H
#define CPP_LIB_SERIAL_H

#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <memory>

#include "cpp-lib/windows/wrappers.h"


namespace cpl {

namespace serial   {

//
// Configure given device with a config string of the form e.g.
// "baud=9600 parity=o data=8 stop=1"
//
// baud   ... One of the standard RS232 baud rates.
// parity ... o (odd), e (even) or N (none).
// data   ... number of data bits.
// stop   ... number of stop bits.
//
// No whitespace other than between key/value pairs is allowed!
//
// The function opens the device and closes it again immediately.
//
// The device is set up for blocking read and write operations.
//

void configure_device( std::string const& name , std::string const& config ) ;


//
// Serial interface with two separate streams for input and output.  I/O
// is blocking, the read buffer size can be set.
//

class tty {
  
  std::unique_ptr< cpl::detail_::tty_impl > impl ;

  tty const& operator=( tty const& ) ;
  tty                 ( tty const& ) ;

public:

  //
  // Open the serial device name, configure it according to config (see
  // configure_device() above).  Read streambuf buffer size will be n.
  // n >= 1.
  //
  // Throws on any errors.
  //

  tty( 
	std::string const& name   , 
	std::string const& config , 
	int const          n = 1024
  ) ;

  // The input and output streams.
  std::istream in  ;
  std::ostream out ;

} ;


} // namespace serial

} // namespace cpl


#endif // CPP_LIB_SERIAL_H
