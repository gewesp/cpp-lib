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
// Component: CRYPT
//

#ifndef CPP_LIB_CRYPT_STREAM_H
#define CPP_LIB_CRYPT_STREAM_H

#include <fstream>
#include <algorithm>
#include <ios>
#include <memory>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "cpp-lib/util.h"


namespace cpl {

namespace crypt    {


//
// Output and input stream buffers implementing CFB for a block cipher CB.
//
// Algorithm:  See Menenzes, Handbook of Applied Cryptography, Algorithm
// 7.17 (p. 231).
//


//
// The encrypting output buffer.
//

template< typename BC >
struct cfb_ostreambuf : std::streambuf {

  //
  // Set the cipher and initialization vector to use.
  //

  cfb_ostreambuf( 
    BC const& c ,
    std::vector< char > const& IV ,
    std::unique_ptr< std::streambuf > sink  
  ) 
  : cipher( c ) ,
    I( IV ) ,
    sink( std::move( sink ) )
  { 

    if( IV.size() != c.block_size() ) 
    { throw std::runtime_error
      ( "cfb: bad initialization vector size" ) ; }

  }

protected:

  //
  // Convention:  x is plaintext, c is ciphertext.
  //

  virtual int_type overflow( int_type x ) {

    // debug version:  one-to-one mapping.
    // return sink->sputc( x ) ;

    if( traits_type::eof() == x ) { return traits_type::eof() ; }

    std::vector< char > O = I ;
    cipher.encrypt_block( O ) ;

    char const t = O[ 0 ] ;
    char const c = x ^ t ;

    // Shift I one char to the right.
    std::copy_backward( I.begin() , I.end() - 1 , I.end() ) ;
    I[ 0 ] = c ;

    return sink->sputc( c ) ;

  }

  virtual int sync() { return sink->pubsync() ; }

private:

  // Members cannot be const for move constructor!
  BC cipher             ;
  std::vector< char > I ;
  std::unique_ptr< std::streambuf > sink ;

} ;


// 
// The decrypting input buffer.  It connects to another input streambuf 
// (which it owns) and decrypts whatever appears there.
// Uses CFB mode, see top comment.
//

template< typename BC >
struct cfb_istreambuf : std::streambuf {

  //
  // Set the cipher, initialization vector and source to use.
  // Owns the source.
  //

  cfb_istreambuf( 
    BC const& c ,
    std::vector< char > const& IV ,
    std::unique_ptr< std::streambuf > source 
  ) 
  : cipher( c      ) ,
    I     ( IV     ) ,
    source( std::move( source ) )
  { 

    setg( 
      buffer + size_pb ,     // beginning of putback area
      buffer + size_pb ,     // read position
      buffer + size_pb       // end position
    ) ;   

    if( IV.size() != c.block_size() ) 
    { throw std::runtime_error
      ( "cfb: bad initialization vector size" ) ; }

  }


protected:

  virtual int_type underflow () {

    // is read position before end of buffer?

    if( gptr() < egptr() ) 
    { return traits_type::to_int_type( *gptr() ) ; }

    /* process size of putback area
     * - use number of characters read
     * - but at most size of putback area
     */
    int const n_pb = 
      std::min( 
        static_cast< long >( gptr() - eback() ) , 
        static_cast< long >( size_pb          ) 
      ) ;

    /* copy up to size_pb characters previously read into
     * the putback area
     */
    std::memmove( buffer + size_pb - n_pb , gptr() - n_pb , n_pb ) ;

    // Read at most buf_size new characters and put them beginning
    // from buffer + size_pb.
    int i ;
    for( i = 0 ; i < buf_size ; ++i ) {

      int_type const c = source->sbumpc() ;

      if( traits_type::eof() == c ) { break ; }

      std::vector< char > O = I ;
#if 0 
      std::cerr 
        << int( O[ 0 ] ) << ' ' << int( O[ 1 ] ) << ' '
        << int( O[ 2 ] ) << ' ' << int( O[ 3 ] ) << ' '
        << int( O[ 4 ] ) << ' ' << int( O[ 5 ] ) << ' '
        << int( O[ 6 ] ) << ' ' << int( O[ 7 ] )
        << '\n' 
      ;
#endif

      cipher.encrypt_block( O ) ;

#if 0
      std::cerr 
        << int( O[ 0 ] ) << ' ' << int( O[ 1 ] ) << ' '
        << int( O[ 2 ] ) << ' ' << int( O[ 3 ] ) << ' '
        << int( O[ 4 ] ) << ' ' << int( O[ 5 ] ) << ' '
        << int( O[ 6 ] ) << ' ' << int( O[ 7 ] )
        << '\n' 
      ;
#endif


      char const t = O[ 0 ] ;
      char const x = traits_type::to_char_type( c ) ^ t ;

      // Shift I one char to the right.
      std::copy_backward( I.begin() , I.end() - 1 , I.end() ) ;
      I[ 0 ] = c ;

      *( buffer + size_pb + i ) = x ;
      // *( buffer + size_pb + i ) = traits_type::to_char_type( c ) ;

    }

    if( 0 == i ) { return traits_type::eof() ; }

    // reset buffer pointers
    setg(
      buffer + size_pb - n_pb , // beginning of putback area
      buffer + size_pb        , // read position
      buffer + size_pb + i      // end of buffer
    ) ;

    return traits_type::to_int_type( *gptr() ) ;

  }

private:

  // 
  // Data buffer:
  //
  // - at most, size_pb characters in putback area plus
  // - at most, buf_size characters in ordinary read buffer
  // 
  
  enum { size_pb  = 4    } ;  // size of putback area
  enum { buf_size = 1024 } ;  // size of the data buffer

  char buffer[ buf_size + size_pb ] ;  // data buffer

  // Cannot be const due to move constructor.
  BC cipher              ;
  std::vector< char > I  ;
  std::unique_ptr< std::streambuf > source ;

} ;



//
// Open a file for reading while decrypting it using the given block 
// cipher in CFB mode with the given initialization vector.
//
// The file is opened by delegation to open_readbuf().
//

template< typename BC >
cpl::util::file::owning_istream< cfb_istreambuf< BC > > open_read(
  BC const& cipher              ,
  std::vector< char > const& IV ,
  std::string const& name       ,
  std::string      & which      ,
  std::vector< std::string > const& path = std::vector< std::string >()
) {

  // Move into a dynamically allocated object for polymorphism.
  std::unique_ptr< std::streambuf > source(
      new std::filebuf( 
          cpl::util::file::open_readbuf( name , which , path ) ) ) ;

  return cpl::util::file::owning_istream< cfb_istreambuf< BC > >(
      cfb_istreambuf< BC >( cipher , IV , std::move( source ) ) ) ;

}


//
// Equivalent to the above function but without information about
// the opened file.
//

template< typename BC >
cpl::util::file::owning_istream< cfb_istreambuf< BC > > open_read(
  BC const& cipher              ,
  std::vector< char > const& IV ,
  std::string const& name  ,
  std::vector< std::string > const& path = std::vector< std::string >()
) { 
  
  std::string dummy ; 
  return open_read( cipher , IV , name , dummy , path ) ; 

}


//
// Open a file for writing while encrypting it using the given block 
// cipher in CFB mode with the given initialization vector.
//
// The file is opened by delegation to open_writebuf().
//

template< typename BC >
cpl::util::file::owning_ostream< cfb_ostreambuf< BC > > open_write(
  BC const& cipher              ,
  std::vector< char > const& IV ,
  std::string const& name 
) {
  
  // Move into a dynamically allocated object for polymorphism.
  std::unique_ptr< std::streambuf > source(
      new std::filebuf( 
          cpl::util::file::open_writebuf( name ) ) ) ;

  return cpl::util::file::owning_ostream< cfb_ostreambuf< BC > >(
      cfb_ostreambuf< BC >( cipher , IV , std::move( source ) ) ) ;

}

} // namespace crypt

} // namespace cpl


#endif // CPP_LIB_CRYPT_STREAM_H
