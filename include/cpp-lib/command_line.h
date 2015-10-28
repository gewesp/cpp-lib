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
// Component: UTIL
//


#ifndef CPP_LIB_COMMAND_LINE_H
#define CPP_LIB_COMMAND_LINE_H


#include <string>
#include <map>
#include <set>
#include <exception>
#include <stdexcept>
#include <utility>


namespace cpl {

namespace util {

/// Option properties defining short name and if argument is required.

struct option_properties {

  /// Initialize.

  option_properties( bool arg , char short_name = 0 )
  : arg( arg ) , short_name( short_name ) {}

  /// If true, the option requires an argument.
  bool arg ;

  /// The short name of the option, zero if none is defined.
  char short_name ;

} ;

/// Property map entry type.

typedef std::pair< std::string , option_properties > opm_entry ;

/// Property map type.  Maps long option names to properties.

typedef std::map< std::string , option_properties > property_map ;

/// Convenience alias.

typedef property_map opm ;

/// Convenience alias.

typedef option_properties opp ;

struct bad_command_line : std::runtime_error {

  bad_command_line( std::string const& what )
  : std::runtime_error( what ) {}

} ;

/// Parse the command line.

/// This structure contains information about
/// - command line options recognized by the program,
/// - the properties of the individual options, and
/// - the actual command line itself.
///
/// It provides functions for
/// - Checking if a particular option is given
/// - Retrieving option arguments based on the option name.
/// - Extracting non-option arguments.
///
/// Option processing stops as soon as
/// - the first non-option argument or
/// - a double dash (<tt>--</tt>)
/// is encountered.

struct command_line {

  /// Initialize property map and parse command line.
  /// \param begin Pointer to the first element of the list of
  /// recognized options.
  /// \param end Pointer to past-the-end element of the list of
  /// recognized options.
  /// \param argv The argument vector (as passed to main()).

  command_line(
    opm_entry const* begin ,
    opm_entry const* end ,
    char const * const * argv
  ) ;


  /// \return True if and only if option is given on the command line.

  /// This function returns true
  /// if and only if the long or short form of the respective option
  /// is given on the command line.
  ///
  /// An exception is thrown if the option isn't recognized.
  /// \param name Long option name.

  bool is_set( std::string const& name ) const ;


  /// \return The argument of command line option name, if given on
  /// the command line.

  /// This function returns the argument of the given command line option.
  /// If the option isn't recognized or doesn't take an argument or isn't
  /// given on the command line, a std::runtime_error containing an appropriate
  /// message is thrown.

  std::string get_arg( std::string const& name ) const ;


  /// \return The argument of command line option name, if given on
  /// the command line, or default_value otherwise.

  /// This function returns the argument of the given command line option.
  /// If the option isn't recognized or doesn't take an argument 
  /// a std::runtime_error containing an appropriate message is thrown.

  std::string get_arg_default( std::string const& name ,
                               std::string const& default_value ) const ;


  /// Convert to state (as in std::istream).

  /// \return True as long as non-option arguments are available for
  /// extraction.

  operator bool() const { return state ; }


  /// Non-option argument extraction.

  /// \retval s The next non-option argument, if available.
  ///
  /// A non-option argument is available if and only if operator bool()
  /// returns true.

  friend command_line& operator>>( command_line& , std::string& s ) ;

private:

  /// Check if option is recognized.

  /// If the option is not recognized, a std::runtime_error is thrown.
  /// \return True if and only if the option requires an argument.

  bool check_option( std::string const& name ) const ;


  /// Non-option arguments available?

  bool state ;

  /// Pointer to current argument.

  char const * const * argv ;


  /// The property map mapping long names to properties.

  const property_map props ;

  /// A ``reverse lookup'' map for mapping short to long option names.

  std::map< char , std::string > short_to_long ;


  /// Which flags are set?

  std::set< std::string > flags ;

  /// Which arguments are given?

  std::map< std::string , std::string > arguments ;
} ;

      
/// Non-option argument extraction.

/// \retval s The next non-option argument, if available.
///
/// A non-option argument is available if operator bool() returns
/// true.
command_line& operator>>( command_line& cl, std::string& s ) ;

} // end namespace util

} // end namespace cpl

#endif // CPP_LIB_COMMAND_LINE_H
