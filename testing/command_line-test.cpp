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

#include <iostream>
#include <stdexcept>
#include <exception>
#include <string>
#include <memory>

#include <cstdlib>
#include <cassert>

#include "cpp-lib/command_line.h"
#include "cpp-lib/util.h"


using namespace cpl::util ;

namespace {

const opm_entry options[] = {

  opm_entry( "list"       , opp( false , 'l' ) ) ,
  opm_entry( "output"     , opp( true  , 'o' ) ) ,
  opm_entry( "fullscreen" , opp( false , 'f' ) ) ,
  opm_entry( "help"       , opp( false , 'h' ) )

} ;


void usage( std::string const& name ) {

  std::cerr << "usage: " << name << " [ options ]\n"
"-l, --list           List some values.\n"
"-o, --output <file>  Write output to <file>.\n"
"-f, --fullscreen     Fullscreen mode.\n"
"-h, --help           Display this message.\n" ;

}


} // anonymous namespace



int main( int const argc , char const * const * const argv ) {

  try {

  if( argc == 1 ) 
  { std::cout << "Type " << argv[ 0 ] << " --help for help.\n" ; return 0 ; }

  command_line cl( options , options + size( options ) , argv ) ;

  if( cl.is_set( "help" ) ) { usage( argv[ 0 ] ) ; return 0 ; }

  if( cl.is_set( "fullscreen" ) )
  { std::cout << "Fullscreen mode.\n" ; }

  if( cl.is_set( "list" ) ) 
  { std::cout << "List some values...\n" ; }

  if( cl.is_set( "output" ) ) 
  { std::cout << "Write output to " << cl.get_arg( "output" ) << '\n' ; }


  } // global try
  catch( std::runtime_error const& e )
  { std::cerr << e.what() << '\n' ; return 1 ; }

  return 0 ;

}
