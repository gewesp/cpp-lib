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
// Platform independent low-level socket functions.
//

#ifndef CPP_LIB_DETAIL_SOCKET_LOWLEVEL_H
#define CPP_LIB_DETAIL_SOCKET_LOWLEVEL_H

#include "cpp-lib/detail/platform_wrappers.h"

namespace cpl {

namespace detail_ {

void throw_socket_error( std::string const& msg ) ;


//
// Enable certain SO_* options (like SO_REUSEADDR, SO_BROADCAST,...)
// or disable again with false
//

template< typename T >
void enable_sockopt
( socketfd_t const fd , int const option, T const& optval ) {

  if(
    ::setsockopt(
      fd ,
      SOL_SOCKET ,
      option ,
      reinterpret_cast< char const* >( &optval ) ,
      sizeof( optval )
    ) == -1
  ) { throw_socket_error( "setsockopt" ) ; }

}

inline void bool_sockopt( 
    socketfd_t const fd , int const option , bool const enable = true ) {
  enable_sockopt<int>( fd , option , enable ) ;
}

inline void time_sockopt(
    socketfd_t const fd , int const option , double const t ) {
  assert( SO_SNDTIMEO == option || SO_RCVTIMEO == option ) ;

  ::timeval const tv = cpl::detail_::to_timeval( t ) ;
  enable_sockopt<::timeval>( fd , option , tv ) ;
}

// socketsend() is a thin layer around send() to abstract away platform
// differences.
//
// SIGPIPE occurs on writing to a socket if the peer has already shut
// it down.  This dangerous behavior seems to be mandated by POSIX.
//
// The machanism for avoiding SIGPIPE is not specified in POSIX
// and hence different across systems:
// * Linux allows to avoid it on a per-send() basis (MSG_NOSIGNAL flag).  
// * MacOS Darwin: Per-socket basis (SO_NOSIGPIPE).
// TODO: Other systems.
inline int socketsend( socketfd_t const fd , char const* const data , 
    std::size_t const size ) {
#ifdef CPP_LIB_PLATFORM_linux
  const int flags = MSG_NOSIGNAL ;
#else
  const int flags = 0 ;
#endif
  return send( fd , data , size , flags ) ;
}

// Platform dependent setup for stream sockets
inline void setup_stream_socket( socketfd_t const fd ) {
#ifdef CPP_LIB_PLATFORM_darwin
  bool_sockopt( fd , SO_NOSIGPIPE ) ;
#else
  static_cast<void>(fd);
#endif
}


} // detail_

} // cpl


#endif // CPP_LIB_DETAIL_SOCKET_LOWLEVEL_H
