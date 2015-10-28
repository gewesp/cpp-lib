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

namespace {

inline int socketclose( socketfd_t const s ) { return closesocket( s ) ; }


const socketfd_t invalid_socket = INVALID_SOCKET ;

// typedef int socklen_t ;

} // anonymous namespace


#define SOCKET_ERROR_MSG cpl::detail_::last_error_message()

// 
// Stuff existing on UNIX but not on Windows.
//

#define h_errno WSAGetLastError()
// #define ECONNREFUSED WSAECONNREFUSED


namespace {

struct winsock_initializer {

  winsock_initializer() {

    const WORD wVersionRequested = MAKEWORD( 2 , 2 ) ;
    WSADATA wsaData ;
  
	// std::cout << "WSAStartup()\n" ;

    if( WSAStartup( wVersionRequested, &wsaData ) )
    { cpl::util::die( "couldn't find suitable winsock.dll" ) ; }

  }

  ~winsock_initializer() {
  
    // std::cout << "WSACleanup()\n" ;
    WSACleanup() ;

  }

} william_h_gates_iii ;

} // anonymous namespace
