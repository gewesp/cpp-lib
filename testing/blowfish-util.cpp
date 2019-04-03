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

//
// UNIX only.
//

#include <iostream>
#include <fstream>
#include <string>

#include <sys/stat.h>

#include <cstring>

#include "cpp-lib/blowfish.h"


using namespace cpl::crypt;

void usage()
{
  std::cout << "Usage:" << std::endl;
  std::cout << "    blowfish encrypt|decrypt -in [file] -out [file] -key [key]" << std::endl;
}

int main(int argC, char* argV[])
{
  enum { encrypt, decrypt } operation;

  std::string cipher;
  std::string decipher;
  std::string szHex;

  std::cout << "Blowfish encryption / decryption utility" << std::endl;
  std::cout << "========================================" << std::endl << std::endl;
  std::cout << "N.B. Can only be used for small files." << std::endl;

  if (argC != 8)
  {
    usage();

    return 1;
  }

  if (std::strcmp(argV[1], "encrypt") == 0)
  {
    operation = encrypt;
  }
  else if (std::strcmp(argV[1], "decrypt") == 0)
  {
    operation = decrypt;
  }
  else
  {
    usage();

    return 1;
  }

  std::string inFile;
  std::string outFile;
  std::string key;

  for (int i=2; i<8; i+=2)
  {
    if (std::strcmp(argV[i], "-in") == 0)
    {
      inFile.assign(argV[i+1]);
    }
    else if (std::strcmp(argV[i], "-out") == 0)
    {
      outFile.assign(argV[i+1]);
    }
    else if (std::strcmp(argV[i], "-key") == 0)
    {
      key.assign(argV[i+1]);
    }
    else
    {
      usage();

      return 1;
    }
  }

  buffer keyBuf(key.begin(), key.end());
  blowfish blowFish(keyBuf);

  struct stat inFileStats;
  if (stat(inFile.c_str(), &inFileStats) != 0)
  {
    usage();

    std::cout << std::endl << "Error getting input file statistics." << std::endl;

    return 1;
  }

  struct stat outFileStats;
  if (stat(outFile.c_str(), &outFileStats) == 0)
  {
    usage();

    std::cout << std::endl << "Error: the output file must not aready exist." << std::endl;

    return 1;
  }

  // open the input and output files
  std::ifstream fin(inFile.c_str(), std::ifstream::binary);
  std::ofstream fout(outFile.c_str(), std::ofstream::binary);
  if (fin.bad() || fout.bad())
  {
    usage();

    std::cout << std::endl << "Note that the input file must exist and the output file must NOT already exist!" << std::endl;

    return 1;
  }

  char *inBuf = new char[inFileStats.st_size+1];
  
  fin.read(inBuf, inFileStats.st_size);
  if (fin.bad())
  {
    std::cout << std::endl << "Error reading input file." << std::endl;

    return 1;
  }

  std::string inStr(inBuf, inFileStats.st_size); 

  if (operation == encrypt)
  {
    buffer raw(inStr.begin(), inStr.end());
    blowFish.encrypt(raw);
    std::string hex = blowfish::char2Hex(raw);
    fout.write(hex.c_str(), hex.length());
  }
  else
  {
    buffer raw = blowfish::hex2Char(inStr);
    blowFish.decrypt(raw);
    fout.write(reinterpret_cast<char *>(&raw[0]), raw.size());
  }

  // close the files
  fin.close();
  fout.close();

  // free up the memory
  delete [] inBuf;

  return 0;
}

