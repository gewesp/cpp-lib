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
#include <exception>
#include <stdexcept>
#include <memory>
#include <vector>

#include "cpp-lib/blowfish.h"
#include "cpp-lib/crypt_stream.h"
#include "cpp-lib/util.h"
#include "cpp-lib/sys/file.h"

#include "test_key.h"


using namespace cpl::crypt       ;
using namespace cpl::util        ;
using namespace cpl::util::file  ;


namespace {

std::string const suffix = ".crypt" ;


void usage( std::string const& prog ) {

  std::cerr 
	<< "Usage: " << prog << "<name>[.crypt]\n" 
	<< "Read ciphertext from <name>.crypt, write plaintext to <name>, OR\n"
	<< "read plaintext from <name>, write ciphertext to <name>.crypt.\n" 
	;

}


} // anonymous namespace



int main( int argc , char const * const * const argv ) {


  if( argc != 2 ) { usage( argv[ 0 ] ) ; return 1 ; }

  std::string const name = argv[ 1 ] ;

  try {

  std::vector< char > const K = key() ;
  std::vector< char > const I = IV () ;

  blowfish bf( K ) ;


  if( name.find( suffix ) == name.size() - suffix.size() ) {

	// Name ends in .bf -> decrypt.
	
	std::string const bname = basename( name , suffix ) ;
	
	auto is = open_read ( bf , I , name  ) ;
	auto os = open_write(          bname ) ;

	char c ;
	while( EOF != ( c = is.get() ) ) { os << c ; }

  } else {

	// Name does not end in .bf -> encrypt.
	
	std::string const cname = name + suffix ;

	auto is = open_read (          name  ) ;
	auto os = open_write( bf , I , cname ) ;

	char c ;
	while( EOF != ( c = is.get() ) ) { os << c ; }

  }

  } catch( std::exception const& e ) { die( e.what() ) ; }

}
