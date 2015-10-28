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

#ifndef CPP_LIB_POSIX_WRAPPERS
#define CPP_LIB_POSIX_WRAPPERS

//
// Somebody already seems to define this...
//
// #define _POSIX_C_SOURCE 199506
//

#include <string>
#include <streambuf>
#include <vector>
#include <ios>

#include <cerrno>

#include "cpp-lib/util.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>

// Hack for some old OS.
#if defined( __APPLE__       ) \
&&  defined( __MACH__        ) \
&& !defined( _BSD_SOCKLEN_T_ )

  #define _BSD_SOCKLEN_T_ int

#endif

//
// Call strerror_exception() with expr as a string if expr is less than
// zero.
//
// Standard, but NOT threadsafe!!
//

#define STRERROR_CHECK( expr )  \
  ( ( expr ) >= 0 ) ?           \
  void( 0 )                     \
  :                             \
  cpl::detail_::strerror_exception( #expr ) ;


namespace cpl {


// 
// Entities not to be used directly by application code.
//

namespace detail_  {


//
// Most UNIX system calls fail (return < 0) with errno EINTR if a signal 
// occurs.  Use like:
//
// int ret ;
// do { ret = syscall() } while( EINTR_repeat( ret ) ) ;
//
// Should really be implemented using boost::lambda...
//

template< typename T > inline bool EINTR_repeat( T const ret ) 
{ return ret < 0 && errno == EINTR ; }


// 
// Throw an exception of the form s + ": " + strerror( errnum ).
//
// If errnum is 0, use errno as errnum.
// 
// Standard, but NOT thread safe!
//

void strerror_exception( std::string const& s , int const errnum = 0 ) ;


// 
// Return the strerror_r() message defined by errnum.  Thread safe.
//

std::string const os_message( int const errnum ) ;


//
// Block the signal s and return the signal set { s }.
//

::sigset_t block_signal( int const s ) ;


//
// An auto_resource_traits for POSIX file descriptors, which are ints.
//

template<> struct auto_resource_traits< int > {

  static int invalid() { return -1 ; }

  static bool valid( int const h ) { return h >= 0 ; }

  static void dispose( int const h ) { ::close( h ) ; } 

} ;


// 
// auto_resource<> specialization for POSIX file descriptors.
//

typedef cpl::util::auto_resource< int > auto_fd ;


//
// Open a file according to om and return its descriptor.  om may be
// in, out or in|out.  File may shared by other open calls for reading.
// Throws on failure.
//

int posix_open
( std::string const& name , 
  std::ios_base::openmode const om = std::ios_base::in | std::ios_base::out ) ;


//
// Remove a file, returning unlink() error code.
//

inline int remove( std::string const& name ) 
{ return ::unlink( name.c_str() ) ; }


//
// Return last modification time [s] (since some fixed epoch) of file
// represented by fd.
//

double modification_time( int const fd ) ;


//
// A struct wrapping a POSIX file.
//

struct file_impl {

  file_impl( std::string const& name ) 
  : fd( posix_open( name , std::ios_base::in ) ) {}

  // Return last access time [s] (since some fixed epoch).
  double modification_time() 
  { return ::cpl::detail_::modification_time( fd.get() ) ; }

private:

  auto_fd fd ;

} ;


//
// Streambuf traits for POSIX files, forwarding to read(2), write(2).
//

struct posix_reader_writer {

  auto_fd fd;
  
  long read ( char      * const buf , long const n ) ;
  long write( char const* const buf , long const n ) ;

  void shutdown_read () {}
  void shutdown_write() {}

} ;

//
// istreambuf<> and ostreambuf<> specializations for POSIX files.
//

typedef cpl::util::istreambuf< posix_reader_writer > 
posix_istreambuf ;

typedef cpl::util::ostreambuf< posix_reader_writer > 
posix_ostreambuf ;


//
// Round a nonnegative time [s] to nearest nanosecond/microsecond and 
// return the respective timespec/timeval structure.
//
// Extensive checks including loss of precision.
//

::timespec const to_timespec( double const& t ) ;
::timeval  const to_timeval ( double const& t ) ;

//
// Convert a timespec/timeval to double.
//

inline double to_double( ::timespec const& t ) 
{ return t.tv_sec + 1e-9 * t.tv_nsec ; }
inline double to_double( ::timeval  const& t )
{ return t.tv_sec + 1e-6 * t.tv_usec ; }


//
// Sockets are ints under POSIX.
//

typedef int socketfd_t ;


//
// POSIX specific socket shutdown.
//

inline void socket_shutdown_read (const socketfd_t s) 
{ ::shutdown(s, SHUT_RD); }
inline void socket_shutdown_write(const socketfd_t s)
{ ::shutdown(s, SHUT_WR); }

} // namespace detail_

} // namespace cpl


#endif // CPP_LIB_POSIX_WRAPPERS
