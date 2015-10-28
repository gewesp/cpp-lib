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
// Component: CRYPT
//

#ifndef CPP_LIB_REGISTRY_CRYPT_H
#define CPP_LIB_REGISTRY_CRYPT_H

#include <string>
#include <stdexcept>

#include "cpp-lib/registry.h"
#include "cpp-lib/crypt_stream.h"
#include "cpp-lib/util.h"


namespace cpl {

namespace util {

//
// Read a registry in encrypted or plaintext form.
//
// First tries to open the file name + suffix and to read it as
// CFB mode (see cfb_istreambuf).
//
// If this fails, tries to open the file name and to read it as
// plaintext.
//
// In both cases, the registry is read with grammar g and the given
// throw_on_redifinition flag.
//

template< typename BC >
void read_encrypted_configuration(
  registry                 & reg  ,
  BC                       & bc   ,
  std::vector< char > const& iv   ,
  std::string         const& name ,
  std::string         const& suffix                = ".crypt"  ,
  grammar             const& g                     = grammar() ,
  bool                const  throw_on_redefinition = true
) {

  using namespace cpl::util::file ;

  try {

    std::string const cname = name + suffix ;
    auto is = cpl::crypt::open_read( bc , iv , cname ) ;

    lexer lex( is , cname ) ;

    reg.read_from( lex , g , throw_on_redefinition ) ;

  } catch( std::exception const& e ) {

    // didn't work, try unencrypted...

    try
    { reg.read_from( name , g , throw_on_redefinition ) ; }
    catch( std::exception const& f ) {

      // both didn't work, report.

      throw std::runtime_error(
          std::string( "read encrypted configuration: " )
        + e.what()
        + "; plaintext configuration: "
        + f.what()
      ) ;

    }

  }

}

} // namespace util

} // namespace cpl

#endif // CPP_LIB_REGISTRY_CRYPT_H
