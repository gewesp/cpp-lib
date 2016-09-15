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

#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>

#include <cmath>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <string.h>

#include "cpp-lib/util.h"
#include "cpp-lib/posix/wrappers.h"
#include "cpp-lib/sys/file.h"
#include "cpp-lib/sys/util.h"


using namespace cpl::detail_ ;
using namespace cpl::util    ;


// Errno is thread safe.
// http://stackoverflow.com/questions/1694164/is-errno-thread-safe
void cpl::detail_::strerror_exception
( std::string const& s , int const errnum ) {

  throw std::runtime_error
  ( s + ": " + std::strerror( errnum ? errnum : errno ) ) ;

}

void cpl::util::file::chdir(std::string const& path) {
  int const res = ::chdir(path.c_str());
  if (0 != res) {
    cpl::detail_::strerror_exception("chdir to " + path, errno);
  }
}

void cpl::util::file::mkdir(std::string const& path, 
                            bool const allow_existing) {
  if (allow_existing && cpl::util::file::exists(path)) {
    return;
  }

  int const res = ::mkdir(path.c_str(), 0777);
  if (0 != res) {
    cpl::detail_::strerror_exception("mkdir " + path, errno);
  }
}

std::string cpl::util::file::getcwd() {
  int constexpr size = 10000;
  char buf[size];
  if (NULL == ::getcwd(buf, size - 1)) {
    cpl::detail_::strerror_exception("get working directory", errno);
  }
  return buf;
}

bool cpl::util::file::exists( std::string const& name ) {
  return 0 == ::access(name.c_str(), R_OK | F_OK);
}

void cpl::util::file::link(
    std::string const& src, std::string const& dest) {
  int const res = ::link(src.c_str(), dest.c_str());
  if (0 != res) {
    cpl::detail_::strerror_exception("link " + src + " to " + dest, errno);
  }
}

void cpl::util::file::symlink(
    std::string const& src, std::string const& dest) {
  int const res = ::symlink(src.c_str(), dest.c_str());
  if (0 != res) {
    cpl::detail_::strerror_exception("symlink " + src + " to " + dest, errno);
  }
}


void cpl::util::file::unlink(std::string const& path, bool ignore_missing) {
  int const res = ::unlink(path.c_str());
  if (0 != res) {
    if (ignore_missing && ENOENT == errno) {
      return;
    }
    cpl::detail_::strerror_exception("unlink (remove) " + path, errno);
  }
}

void cpl::util::file::rename(
    std::string const& src, std::string const& dest) {
  int const res = ::rename(src.c_str(), dest.c_str());
  if (0 != res) {
    cpl::detail_::strerror_exception("rename " + src + " to " + dest, errno);
  }
}

cpl::util::file::dir_sentry::dir_sentry()
: stored{cpl::util::file::getcwd()} {}

cpl::util::file::dir_sentry::~dir_sentry() {
  cpl::util::file::chdir(stored);
}

#if 0

// TODO: Switch to reentrant version...

int const OS_ERROR_BUFSIZE = 1024 ;

std::string const cpl::detail_::os_message( int const errnum ) {

  std::vector< char > v( OS_ERROR_BUFSIZE ) ;

  int const res = ::strerror_r( errnum , &v[ 0 ] , v.size() ) ;

  if( !res ) { return &v[ 0 ] ; }

  return "strerror_r() message too long" ;

}
#endif


double cpl::detail_::modification_time( int const fd ) {

  struct stat buf ;

  if( ::fstat( fd , &buf ) < 0 ) { strerror_exception( "fstat" ) ; }

  return buf.st_mtime ;

}


int cpl::detail_::posix_open( 
  std::string             const& name ,
  std::ios_base::openmode const  om 
) {

  if( 0 == om || om != ( om & ( std::ios_base::out | std::ios_base::in ) ) )
  { throw std::runtime_error( "bad file open mode" ) ; }

  int flags = 0 ;
  if     ( std::ios_base::out == om ) { flags = O_WRONLY ; }
  else if( std::ios_base::in  == om ) { flags = O_RDONLY ; }
  else                                { flags = O_RDWR   ; }

  int fd = ::open( name.c_str() , flags ) ;

  if( fd < 0 ) { strerror_exception( "open " + name ) ; }

  return fd ;

}


void cpl::util::sleep( double const& t ) {

  assert( t >= 0 ) ;

  ::timespec const tt = to_timespec( t ) ;

  if( -1 == ::nanosleep( &tt , 0 ) )
  { strerror_exception( "sleep" ) ; }

}


double cpl::util::time() {

  ::timeval t ;

  int const err = ::gettimeofday( &t , 0 ) ;
  if( err ) { strerror_exception( "gettimeofday()" , err ) ; }
  return to_double( t ) ;

}


::sigset_t cpl::detail_::block_signal( int const s ) {

  ::sigset_t ret ;

  // sigemptyset() and sigaddset() may be macros!
  STRERROR_CHECK( sigemptyset( &ret     ) ) ;
  STRERROR_CHECK( sigaddset  ( &ret , s ) ) ;

  int const err = ::pthread_sigmask( SIG_BLOCK , &ret , 0 ) ;
  if( err ) { strerror_exception( "pthread_sigmask()" , err ) ; }

  return ret ;
 
}


long cpl::detail_::posix_reader_writer::read(char* const buf, long const n) {
  
  long ret ;
  do { ret = ::read( fd.get() , buf , n ) ; } 
  while( EINTR_repeat( ret ) ) ;

  assert( -1 <= ret      ) ;
  assert(       ret <= n ) ;

  return ret ;

}


long cpl::detail_::posix_reader_writer::write(
    char const* const buf, long const n) {
  
  long ret ;

  // TODO: partial send
  do { ret = ::write( fd.get() , buf , n ) ; } 
  while( EINTR_repeat( ret ) ) ;

  if( ret < 0 ) 
  { return -1 ; }

  assert( n == ret ) ;
  return ret ;

}

::timespec const cpl::detail_::to_timespec( double const& t ) {

  ::timespec ret ;
  to_fractional( t , 1000000000 , ret.tv_sec , ret.tv_nsec ) ;
  return ret ;

}

::timeval const cpl::detail_::to_timeval( double const& t ) {

  ::timeval ret ;
  to_fractional( t , 1000000 , ret.tv_sec , ret.tv_usec ) ;
  return ret ;

}
