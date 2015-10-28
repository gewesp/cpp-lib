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


#include "cpp-lib/sys/file.h"
#include "cpp-lib/platform/wrappers.h"


using namespace cpl::util::file ;
using namespace cpl::detail_ ;


cpl::util::file::File::File( std::string const& name )
: impl( new file_impl( name ) ) {}

cpl::util::file::File::~File() {}

double cpl::util::file::File::modification_time()
{ return impl->modification_time() ; }


bool cpl::util::file::file_name_watcher::modified() {

  double tt = 0 ;
  try {
    tt = File( name ).modification_time() ;
  } catch( std::exception const& e ) {
    // In case the file goes away, we expect it to reappear shortly and
    // report no modification.
    return false ;
  }

  if( tt > t ) { t = tt ; return true ; }
  else         { return false ; }

}


bool cpl::util::file::file_watcher::modified() {

  double const tt = f.modification_time() ;

  if( tt > t ) { t = tt ; return true ; }
  else         { return false ; }

}

bool cpl::util::file::file_name_queue::add( std::string const& filename ) {
  
  q.push_back( filename ) ;
  if ( static_cast< long >( q.size() ) > maxsize ) {
    std::string const to_remove = std::move( q.front() ) ;
    q.pop_front() ;
    return cpl::detail_::remove( to_remove ) ;
  } else {
    return false ;
  }
}
