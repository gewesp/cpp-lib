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

//
// Blowfish.h Header File
//
//    BLOWFISH ENCRYPTION ALGORITHM
//
//    Encryption and Decryption of Byte Strings using the Blowfish Encryption Algorithm.
//    Blowfish is a block cipher that encrypts data in 8-byte blocks. The algorithm consists
//    of two parts: a key-expansion part and a data-ancryption part. Key expansion converts a
//    variable key of at least 1 and at most 56 bytes into several subkey arrays totaling
//    4168 bytes. Blowfish has 16 rounds. Each round consists of a key-dependent permutation,
//    and a key and data-dependent substitution. All operations are XORs and additions on 32-bit words.
//    The only additional operations are four indexed array data lookups per round.
//    Blowfish uses a large number of subkeys. These keys must be precomputed before any data
//    encryption or decryption. The P-array consists of 18 32-bit subkeys: P0, P1,...,P17.
//    There are also four 32-bit S-boxes with 256 entries each: S0,0, S0,1,...,S0,255;
//    S1,0, S1,1,...,S1,255; S2,0, S2,1,...,S2,255; S3,0, S3,1,...,S3,255;
//
//    The Electronic Code Book (ECB), Cipher Block Chaining (CBC) and Cipher Feedback modes
//    are used:
//
//    In ECB mode if the same block is encrypted twice with the same key, the resulting
//    ciphertext blocks are the same.
//
//    In CBC Mode a ciphertext block is obtained by first xoring the
//    plaintext block with the previous ciphertext block, and encrypting the resulting value.
//
//    In CFB mode a ciphertext block is obtained by encrypting the previous ciphertext block
//    and xoring the resulting value with the plaintext
//
//    The previous ciphertext block is usually stored in an Initialization Vector (IV).
//    An Initialization Vector of zero is commonly used for the first block, though other
//    arrangements are also in use.

#ifndef CPP_LIB_BLOWFISH_H
#define CPP_LIB_BLOWFISH_H

#include <vector>
#include <string>
#include <exception>
#include <stdexcept>

#include <cassert>


namespace cpl
{
  namespace crypt
  {

    //Block Structure
    class block
    {
    public:
      unsigned int m_uil;
      unsigned int m_uir;

	    //Constructors
	    block(unsigned int l=0, unsigned int r=0) : m_uil(l), m_uir(r) {}
	    //Copy Constructor
	    block(const block& roBlock) : m_uil(roBlock.m_uil), m_uir(roBlock.m_uir) {}
	    block& operator^=(block& b) { m_uil ^= b.m_uil; m_uir ^= b.m_uir; return *this; }

    };


	//Semi-Portable Byte Shuffling.
	// C must be a character type.
	template< typename C >
	inline block const vectorToBlock(const std::vector< C >& bytes )
	{

	  assert( 8 == bytes.size() ) ;
	  assert( 1 == sizeof( C ) ) ;

		unsigned int y;
	  int shift = 24;

	  block b;

	  b.m_uil = 0;
	  for (int i=0; i<4; i++)
	  {
		y = static_cast< unsigned char >( bytes[i] ) ;
		y <<= shift;
		b.m_uil |= y;

		shift -=8;
	  }

	  shift = 24;
	  b.m_uir = 0;
	  for (int i=0; i<4; i++)
	  {
		y = static_cast< unsigned char >( bytes[i+4] );
		y <<= shift;
		b.m_uir |= y;

		shift -=8;
	  }

	  return b;
	}

	template< typename C >
	inline std::vector< C > const blockToVector(const block &b)
	{
	  assert( 1 == sizeof( C ) ) ;

	  unsigned int y;
	  std::vector< C > out(8, '\0');
	  
	  int shift = 0;
	  for (int i=0; i<4; i++)
	  {
		y = b.m_uir >> shift;
		out[7-i] = y & 0xff ;

		shift += 8;
	  }
	  
	  shift = 0;
	  for (int i=0; i<4; i++)
	  {
		y = b.m_uil >> shift;
		out[3-i] = y & 0xff ;

		shift += 8;
	  }

	  assert( 8 == out.size() ) ;
	  return out;

	}
 


    typedef std::vector<unsigned char> buffer;
    enum blowfishAlgorithm { ECB=0, CBC=1, CFB=2 };

    class blowfish
    {
    private:
	    //The Initialization Vector, by default {0, 0}
	    block m_oChain0;
	    block m_oChain;
	    unsigned int m_auiP[18];
	    unsigned int m_auiS[4][256];
	    static const unsigned int scm_auiInitP[18];
	    static const unsigned int scm_auiInitS[4][256];

	    unsigned int F(unsigned int ui)
      {
	      return ((m_auiS[0][byte(ui>>24)] + m_auiS[1][byte(ui>>16)]) ^ m_auiS[2][byte(ui>>8)]) + m_auiS[3][byte(ui)];
      }

      //Extract low order byte
      unsigned char byte(unsigned int ui)
      {
	      return (unsigned char)(ui & 0xff);
      }

      buffer encodeInt(unsigned long integer);
      unsigned long decodeInt(const buffer &buf);

      block bytesToBlock(const buffer &bytes);
      buffer blockToBytes(const block &b);

      void encrypt(block&);
	    void decrypt(block&);

	  void init( const buffer &ucKey ) ;

      static std::string convertChar2Hex(const unsigned char ch);
      static unsigned char convertHex2Char(const unsigned char* const szHex);

    public:
      //Constructor - Initialize the P and S boxes for a given Key
	  // ucKey must have at least one element.  At most 56 elements are
	  // used.
      blowfish(const buffer &ucKey, const block& roChain = block(0UL,0UL));
      
	  // As above, but for char.
      blowfish( 
		std::vector< char > const& ucKey, 
	    const block& roChain = block(0UL,0UL)
	  ) ;

	  //As above, but for string.
	  blowfish
	  ( std::string const& key , const block& roChain = block(0UL,0UL) ) ;

	    //Resetting the chaining block
	    void resetChain() { m_oChain = m_oChain0; }

	  // Encrypt/Decrypt Buffer in Place.  Pads with zeroes if block
	  // length is not a multiple of 8.  encrypt() prepends the size of
	  // the block.
      void encrypt(buffer &buf, int iMode=ECB);
      void decrypt(buffer &buf, int iMode=ECB);


	  //
	  // Encrypt an 8-byte vector.
	  //

	  template< typename C > void encrypt_block( std::vector< C >& v ) {

		assert( 1 == sizeof( C ) ) ;
		if( v.size() != block_size() )
		{ throw std::runtime_error( "bad block size" ) ; }

		block b = vectorToBlock( v ) ;
		encrypt( b ) ;
		v = blockToVector< C >( b ) ;

		assert( 8 == v.size() ) ;

	  }

	  //
	  // Decrypt an 8-byte vector.
	  //

	  template< typename C > void decrypt_block( std::vector< C >& v ) {

		assert( 1 == sizeof( C ) ) ;
		if( v.size() != block_size() )
		{ throw std::runtime_error( "bad block size" ) ; }

		block b = vectorToBlock( v ) ;
		decrypt( b ) ;
		v = blockToVector< C >( b ) ;

		assert( 8 == v.size() ) ;

	  }

	  // Return block length.
	  int blockLength() const { return 8 ; }

	  // Synonym.
	  unsigned long block_size() const { return blockLength() ; }

      static std::string char2Hex(const buffer &charStr);
      static buffer hex2Char(const std::string &hexStr);

    };
  }
}
#endif // CPP_LIB_BLOWFISH_H
