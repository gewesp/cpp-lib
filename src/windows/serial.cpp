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

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "cpp-lib/windows/wrappers.h"
#include "cpp-lib/serial.h"

using namespace cpl::detail_ ;

namespace {

// Buffer sizes
int const  in_queue = 1024 ;
int const out_queue = 1024 ;

//
// According to the information on www.msdn.com (Communications
// Resources), changes via SetCommState() are persistent across multiple
// open/close calls.
//

void configure_device
( auto_handle const& fd , std::string const& config ) {

  COMMTIMEOUTS cto ;
  std::memset( &cto , 0 , sizeof( cto ) ) ;

  // No idea why, but some trial/error indicates that these values might
  // be the best ones...
  cto.ReadIntervalTimeout          = MAXDWORD ;
  cto.ReadTotalTimeoutMultiplier   = MAXDWORD ;
  cto.ReadTotalTimeoutConstant     = MAXDWORD - 1 ;
  cto.WriteTotalTimeoutMultiplier  = MAXDWORD ;
  cto.WriteTotalTimeoutConstant    = MAXDWORD ;

  if( !SetCommTimeouts( fd.get() , &cto ) )
  { cpl::detail_::last_error_exception( "SetCommTimeouts" ) ; }
  

  DCB dcb ;
  std::memset( &dcb , 0 , sizeof( dcb ) ) ;

  // SetCommState() documentation says that we should call
  // GetCommState() first to get a reasonably initialized DCB...
  if( !GetCommState( fd.get() , &dcb ) )
  { cpl::detail_::last_error_exception( "GetCommState" ) ; }

  if( !BuildCommDCB( config.c_str() , &dcb ) ) 
  { cpl::detail_::last_error_exception( "BuildCommDCB" ) ; }

  if( !SetCommState( fd.get() , &dcb ) )
  { cpl::detail_::last_error_exception( "SetCommState" ) ; }


  // if( !SetupComm( fd.get() , in_queue , out_queue ) )
  // { cpl::detail_::last_error_exception( "SetupComm" ) ; }

}

} // end anon namespace


void cpl::serial::configure_device
( std::string const& name , std::string const& config ) {

  auto_handle const fd( windows_open( name ) ) ;

  ::configure_device( fd , config ) ;

}


cpl::serial::tty::tty
( std::string const& name , std::string const& config , int const n ) 
: impl( new tty_impl( windows_open( name ) , n ) ) ,
  in ( &impl->ibuf ) ,
  out( &impl->obuf ) 
{ ::configure_device( impl->fd , config ) ; }
