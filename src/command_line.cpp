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

using namespace cpl::util;

cpl::util::command_line::command_line(

  opm_entry const* beg ,
  opm_entry const* end ,
  char const * const * argv_

)
: state( true ) , argv( argv_ ) , props( beg , end )
{

  for( property_map::const_iterator i = props.begin() ;
       i != props.end() ;
       ++i ) {
    if( short_to_long.count( i->second.short_name ) ) {
      throw bad_command_line( "short option name used twice" ) ;
    } else {
      if( i->second.short_name ) {
        short_to_long[ i->second.short_name ] = i->first ;
      }
    }
  }


  // skip invocation name

  while( *++argv ) {

    std::string s = *argv ;


    // non-option argument?

    if( s.size() <= 1 ) break ;

    if( s[ 0 ] != '-' ) break ;


    std::string name ;

    if( s.size() == 2 ) {

      // stop option processing on --
      if( s[ 1 ] == '-' ) {
        ++argv ;
        break ;
      }


      // else we have a short option

      std::map< char , std::string >::const_iterator i =
	short_to_long.find( s[ 1 ] ) ;

      if( i == short_to_long.end() )
	throw bad_command_line( "unknown option: " + s ) ;


      name = i->second ;

    }

    else {

      if( s[ 1 ] != '-' )
	throw bad_command_line( "sorry: option flags cannot be combined" ) ;

      // long option

      name.assign( s.begin() + 2 , s.end() ) ;

    }


    property_map::const_iterator i = props.find( name ) ;

    if( i == props.end() )
      throw bad_command_line( "unknown option: " + s ) ;


    if( i->second.arg ) {

      // option requires argument

      if( *++argv ) arguments[ name ] = *argv ;
      else
	throw bad_command_line( "option " + s + " requires an argument" ) ;


    }

    // flag option

    else flags.insert( name ) ;

  }


}

command_line& cpl::util::operator>>( command_line& cl , std::string& s )
{
  if( !*cl.argv ) cl.state = false ;
  if( !cl.state ) return cl ;

  s = *cl.argv ;
  ++cl.argv ;
  return cl ;
}


bool cpl::util::command_line::is_set( std::string const& opt ) const
{

  bool arg = check_option( opt ) ;

  if( arg ) return arguments.count( opt ) != 0;
  else      return flags.count( opt ) != 0;

}


std::string cpl::util::command_line::get_arg( std::string const& opt ) const
{

  bool arg = check_option( opt ) ;

  if( !arg )
    throw std::runtime_error( "oops: option " + opt + " is a flag" ) ;

  std::map< std::string , std::string >::const_iterator i =
    arguments.find( opt ) ;

  if( i == arguments.end() )
    throw std::runtime_error( "option --" + opt + " required" ) ;

  return i->second ;

}


std::string cpl::util::command_line::get_arg_default( 
    std::string const& opt , std::string const& default_value ) const {

  if( !check_option( opt ) )
    throw std::runtime_error( "oops: option " + opt + " is a flag" ) ;

  std::map< std::string , std::string >::const_iterator const i =
    arguments.find( opt ) ;

  if( arguments.end() == i ) {
    return default_value;
  } else {
    return i->second ;
  }
}


bool cpl::util::command_line::check_option( std::string const& opt ) const
{

  property_map::const_iterator i = props.find( opt ) ;

  if( i == props.end() )
    throw std::runtime_error( "oops: option " + opt + " doesn't exist" ) ;

  return i->second.arg ;

}
