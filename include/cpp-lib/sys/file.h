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


#ifndef CPP_LIB_FILE_SYS_H
#define CPP_LIB_FILE_SYS_H

#include <deque>
#include <memory>
#include <string>

#include "cpp-lib/util.h"
#include "cpp-lib/assert.h"

namespace cpl { namespace detail_ { struct file_impl ; } }


namespace cpl {

namespace util {

namespace file {

// Utilities forwarding to the respective system functions with
// exceptions in case of errors.

// Changes working directory for this process, throws in case of errors.
void chdir( std::string const& ) ;

// Gets current working directory (absolute path name).
std::string getcwd() ;

// Returns true if file exists and is readable
bool exists( std::string const& ) ;

// Links source to destination (hard link), throws in case of errors.
// Destination must not exist.
void link( std::string const& source , std::string const& destination ) ;

// Links source to destination (symbolic link), throws in case of errors.
// Destination must not exist.
void symlink( std::string const& source , std::string const& destination ) ;

// Removes the specified file, throws in case of errors.
// If ignore_missing is set to true, a nonexistent file is *not*
// considered an error.
void unlink( std::string const& , bool ignore_missing = false ) ;

// Renames source to destination (hard link), throws in case of errors.
// If destination exists, it will be overwritten.
void rename( std::string const& source , std::string const& destination ) ;

// A sentry object to reset the working directory to what it was
// at construction time.
struct dir_sentry {
  // Stores current working directory
  dir_sentry();

  // Returns working directory at construction time
  std::string const& directory() const { return stored ; } 

  // Restores working directory to what it was at construction time
  ~dir_sentry();

private:
  std::string const stored;
};


struct File {

  // Open the file.  Subsequent opens are possible during our lifetime.
  File( std::string const& name ) ;

  // Return last access time [s] (since some fixed epoch).
  double modification_time() ;

  ~File() ;

private:

  File( File const& ) ;

  std::unique_ptr< cpl::detail_::file_impl > impl ;

} ;


//
// An object watching the modification status of a file.
//
// (Necessary because Windows doesn't correctly report modification
// times once file is opened.)
//

struct file_name_watcher {

  file_name_watcher( std::string const& name )
  : name( name ) , t( File( name ).modification_time() ) {}

  // True iff the file was written to since last modified() call.  If
  // the file no longer exists, returns false.  If it reappears on a
  // later call, returns true.
  bool modified() ;

private:

  std::string const name ;
  double t ;

} ;


//
// An object watching the modification status of a file.
//

struct file_watcher {

  file_watcher( std::string const& name )
  : f( name ) , t( f.modification_time() ) {}

  // True iff the file was written to since last modified() call.
  bool modified() ;

private:

  File f ;
  double t ;

} ;

} // namespace file

} // namespace util

} // namespace cpl



#endif // CPP_LIB_FILE_SYS_H
