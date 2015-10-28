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

#include "cpp-lib/registry.h"
#include "cpp-lib/registry-crypt.h"

#include "cpp-lib/assert.h"
#include "cpp-lib/blowfish.h"
#include "cpp-lib/gnss.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/util.h"
#include "cpp-lib/sys/network.h"
#include "cpp-lib/sys/util.h"

#include "test_key.h"


std::string const conf_file        = "registry-test.conf" ;
std::string const conf_file_matlab = "registry-test.m"    ;

using namespace cpl::util          ;
using namespace cpl::util::network ;
using namespace cpl::math          ;
using namespace cpl::crypt         ;


void output( registry const& reg ) {

  //
  // The get< T > template returns the value (which must be of type T)
  // associated with the key.
  //

  std::cout << "logging to "   << reg.get< std::string >( "logfile" ) << '\n' ;
  std::cout << "using engine " << reg.get< std::string >( "engine"  ) << '\n' ;
  std::cout << "using engine " << reg.get_default( "engine" , "rolls-royce" ) 
            << '\n' ;

  std::cout << "Default for (undefined) eau_de_cologne: "
            << reg.get_default( "eau_de_cologne" , 4711.0 ) << '\n' ;

  //
  // There are various check_xyz() members which check that the value
  // associated with the key has certain properties (e.g., is a bool,
  // is a number, ...)
  //

  if( reg.is_set( "flag" ) && reg.check_bool( "flag" ) ) 
  { std::cout << "Fly the flag!!!\n" ; }

  if( reg.get_default( "flag" , false ) )
  { std::cout << "Fly the flag!!!\n" ; }

  std::cout << cpl::gnss::lat_lon_from_registry(reg, "coordinate") << std::endl;

  std::string const host = reg.get< std::string >( "host_2" ) ;

  //
  // check_long() checks for an integer in a given range.
  //

  long const port = check_long( reg.check_nonneg( "port" ) , 0 , 65535 ) ;
  std::cout << "port = " << port << '\n' ;


  std::cout << "pi = " << reg.check_positive( "magic_constant" ) << '\n' ;

  auto const vM = reg.check_vector_double( "matrix" , 9 ) ;
  auto const M = cpl::matrix::to_matrix( vM , 3 , 3 ) ;

  std::cout << "matrix =\n" << M << std::endl ;

  // Yes, you can use a reference here!

  std::vector< boost::any > const& v = reg.check_vector_any( "list" , 3 ) ;

  //
  // The convert< T > template can be used to access the value stored
  // in a boost::any.  It will throw a human-readable error message if
  // the stored object's type does not match T.
  //

  try { 
    
    std::string const& s = convert< std::string >( v[ 1 ] ) ;
    std::cout << "second element of list is: " << s << '\n' ;

  }
  catch( std::runtime_error const& e ) { 
    
    // 
    // reg.key_defined_at( key ) returns a message indicating where key 
    // was defined.
    //

    throw std::runtime_error
    ( "second element of " + reg.key_defined_at( "list" ) + ": " + e.what() ) ; 
  
  }

}


cpl::util::registry test_istringstream(
    cpl::util::grammar const& gr,
    std::string const& configtext) {
  std::istringstream iss{configtext};
  return cpl::util::registry{iss, gr};
}

void test_regression1() {
  cpl::util::grammar gr;
  gr.parser_style.comma_style = cpl::util::comma_optional;

  cpl::util::registry const reg = 
      test_istringstream(gr, "foo={\"foo\" \"bar\"}");
  reg.check_vector_string("foo");
}

// Lexer bug, identifier with '_' at EOF
// Reason was an if(!is) for a std::istream* is (i.e., testing
// the pointer for validity, not the stream.... Arghh...)
void test_regression2() {
  std::istringstream iss("ident_");
  cpl::util::lexer l{iss};
  cpl::util::expect(l, cpl::util::IDENT);
  always_assert(l.string_value() == "ident_");
  cpl::util::expect(l, cpl::util::END);
}

void test_regression3() {
  std::istringstream iss("ident_ ");
  cpl::util::lexer l{iss};
  cpl::util::expect(l, cpl::util::IDENT);
  always_assert(l.string_value() == "ident_");
  cpl::util::expect(l, cpl::util::END);
}

void test_eof1() {
  std::istringstream iss("}");
  cpl::util::lexer l{iss};
  cpl::util::expect(l, cpl::util::RB);
  cpl::util::expect(l, cpl::util::END);
}

void test_unterminated_string() {
  // Unterminated string
  std::istringstream iss("\"hello, world [notice missing closing quote]");
  cpl::util::lexer l{iss};
  // verify_throws doesn't seem to be able to infer the expect() function
  // to use...
  try {
    cpl::util::expect(l, cpl::util::STRING);
  } catch (std::runtime_error const& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}


int main() {

  try {

  // TODO: The two below fails currently, the first with
  // memory overrun.  Urgent fix is necessary.
  test_unterminated_string();
  test_regression2();
  test_regression3();
  test_regression1();
  test_eof1();

  registry reg ;
  
  blowfish bf( key() ) ;
  read_encrypted_configuration( reg , bf , IV() , conf_file , ".crypt" ) ;
  std::cout << conf_file + ".crypt" << " or " << conf_file << ":\n" ;
  output( reg ) ;

  reg.clear() ;
  std::cout << conf_file + ".var" << ":\n" ;
  reg.read_from( conf_file + ".var" , config_style() ) ;
  output( reg ) ;

  reg.clear() ;
  std::cout << "conf_file_matlab" << ":\n" ;
  reg.read_from( conf_file_matlab , matlab_style() , false ) ;
  std::cout 
	<< "string = '" 
	<< reg.get< std::string >( "string" ) 
	<< "'\n"
	<< "list has " 
	<< reg.check_vector_double( "list" ).size() 
	<< " elements.\n"
	;

  expression const& e1 = reg.get< expression >( "expression1" ) ;
  std::cout << "expression1 has head "
            << e1.head
            << " and "
            << e1.tail.size()
            << " arguments.\n"
  ;

  expression const& e2 = reg.get< expression >( "expression2" ) ;
  std::cout << "expression2 has head "
            << e2.head
            << " and "
            << e2.tail.size()
            << " arguments.\n"
  ;

  } catch( std::runtime_error const& e )
  { die( e.what() ) ; }

}
