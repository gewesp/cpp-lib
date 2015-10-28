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
// Component: SYSUTIL
//

//
// Header for functionality that has an OS-independent interface but
// needs to be implemented in an OS specific way.
//
// Includes:
//   time(), sleep()
//   reboot(), poweroff()
//   sleep_scheduler
//   watched_registry
//


#ifndef CPP_LIB_UTIL_SYS_H
#define CPP_LIB_UTIL_SYS_H

#include "cpp-lib/util.h"
#include "cpp-lib/registry.h"
#include "cpp-lib/sys/file.h"

namespace cpl {

namespace util {

//
// Sleep for the specified time [s].  Accuracy is system-dependent,
// typically in the range of 10 milliseconds.
//

void sleep( double const& ) ;

//
// Returns the time [s] since some fixed or variable epoch.  On
// Unix, the epoch is January 1st, 1970.  It may also be
// system startup, however.
//
// Accuracy is system-dependent typically in the range of 10 
// milliseconds.
//
// TODO: Use C++11 chrono?
//

double time() ;

//
// Terminate all processes and reboot.
//

void reboot() ;

//
// Terminate all processes and switch power off.
//

void poweroff() ;

//
// A scheduler that uses 10ms-precision sleeps to achive firings
// at defined intervals relative to t_0.
//
// Clock base: cpl::util::time() (10ms precision since 1970)
//

struct sleep_scheduler {

  // Fire at t_0 + i * dt.
  sleep_scheduler( double const& dt , 
                   double const& t_0 = cpl::util::time() )
  : dt    ( dt     ) ,
    hz    ( 1 / dt ) ,
    t_0   ( t_0    ) ,
    n_next( -std::numeric_limits< double >::max() )
  { always_assert( 0 < dt ) ; }

  // Returns time of current invocation, t_0 + k * dt
  // for an integer k.
  double wait_next() ;

  double time() const { return cpl::util::time() ; }

private:

  double const dt  ;
  double const hz  ;
  double const t_0 ;

  // Actually an integer (except directly after construction)
  double n_next ;

} ;


//
// A registry with the ability to bind to a file, detect changes and
// re-read it in this case.
//

struct watched_registry : registry {

  //
  // Read from named file and bind to the file name for update checking.
  //

  void read_from_and_watch(
    std::string const& name                               ,
     lexer_style_t const&  lexer_style = lexer_style_t () ,
    parser_style_t const& parser_style = parser_style_t() ,
    bool throw_on_redefinition = true
  ) {

    ls =  lexer_style ;
    ps = parser_style ;

    w.reset( new cpl::util::file::file_name_watcher( name ) ) ;

    registry::read_from
    ( name , lexer_style , parser_style , throw_on_redefinition ) ;

  }

  void read_from_and_watch(
    std::string const& name                ,
    grammar     const& g       = grammar() ,
    bool throw_on_redefinition = true
  ) {

    read_from_and_watch
    ( name , g.lexer_style , g.parser_style , throw_on_redefinition ) ;

  }


  //
  // Re-read the registry if a change in the file has been detected.
  // If this is the case, the function returns true and the caller
  // will typically want to re-initialize parameters based on the changed
  // values.
  //
  // A call to this function is only valid if any of the
  // read_from_and_watch() has been called previously and the file
  // could be opened.
  //

  bool read_if_modified() {

    if ( !w.get() ) {
      throw std::runtime_error( 
          "watched_registry: No file to watch or I/O error" );
    }

    if( w->modified() ) {

      registry::read_from( last_filename() , ls , ps , false ) ;
      return true ;

    }

    return false ;

  }

  bool active() const { return w.get(); }

private:

   lexer_style_t ls ;
  parser_style_t ps ;

  std::unique_ptr< cpl::util::file::file_name_watcher > w ;

} ;


} // namespace util

} // namespace cpl


#endif // CPP_LIB_UTIL_SYS_H
