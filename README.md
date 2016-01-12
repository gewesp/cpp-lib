# README for cpp-lib

## What is it?

  Cpp-lib is a general purpose library of ISO C++ functions and classes.
It is provided free of charge and without any implied warranty by KISS 
Technologies GmbH, Switzerland under an Apache license.

## Documentation

  Documentation is by comments in the header files, we may move to doxygen
in the future.  See also the tests for example usage.


## Components

Cpp-lib is organized into the components listed below.

Each header file has a comment of the form

  `// Component: <component name>`

at its top.  UNSUPPORTED is used for functions used only internally
or not ready for production.

### AERONAUTICS

- Open Glider Network (APRS and DDB) client
- IGC file parsing

### CGI

- CGI parameter parsing and URI unescaping

### CRYPT

- A block cipher (blowfish).
- A streambuf filter for transparent reading and writing of encrypted
  files based on arbitrary block ciphers in cipher feedback mode (CFB).

### DISPATCH

- A dispatch queue, modeled after Grand Central Dispatch (GCD)

### GNSS

- Latitude/Longitude/Altitude data structure and related functions.
- Conversion between WGS84 and MSL altitudes (Earth Geoid Model).
- A geographic database with spatial search and openAIP input
  (suitable for airport reference points).

### HTTP

- A very simple HTTP client

### MAP

- Web Mercator tile mapping


### MATH

- Some components are based on the [Eigen] [10] matrix library.
- 3-dimensional geometry, quaternions, Euler angles, etc.
- ODE solvers and some state-space modeling blocks.
- Unconstrained multidimensional minimization with gradients (a C++ version
  of [minimize] [4]).
- Unconstrained multidimensional minimization without gradients (see
  [Nelder/Mead] [5], [downhill simplex][6], [Wolfram] [7]).
- A spatial index based on boost::rtree.


### NETWORK

- Easy TCP/IP and UDP connections.  Iostreams implementation for TCP 
  streams.  See README-networking.md for more information.
- A framework for running TCP services, optionally using threads.

### TABLE

- Arbitrary dimensional tables with dimension defined at runtime.
- A choice of algorithms for multidimensional interpolation.

### REALTIME

- Realtime scheduling.

### REGISTRY

- A versatile configuration file parser with flexible syntax including
  arbitrarily nested lists.  Configurable grammar for various
  pre-existing configuration file styles (C style, Matlab style,
  shell style, ...).  Transparent reading of encrypted files.

### SYSUTIL

- Iostream compatible logging to syslog(3)
- File modification time, alerts on modified files and a registry version with
  on-the-fly reloading of modified files.
- Simple timekeeping and sleep() calls.
- Powering off and rebooting the computer.
- A powerful syslog(3)-based logging facility.

### UTIL

- A class for command line parsing.  Snpports long and short-name
  options with or without arguments.
- Generic stream buffer templates.
- Resource handler templates.
- Physical constants and units.

### VARLIST

- Bind names to variables and provide serialization.


### Namespaces/component association

* `cpl::crypt`           : CRYPT
* `cpl::dispatch`        : DISPATCH
* `cpl::gnss`            : GNSS
* `cpl::igc`             : AERONAUTICS
* `cpl::ogn`             : AERONAUTICS
* `cpl::map`             : MAP
* `cpl::math`            : MATH
* `cpl::util`            : UTIL, VARLIST, REGISTRY, REALTIME, SYSUTIL
* `cpl::units`           : UTIL
* `cpl::util::container` : UTIL
* `cpl::util::file`      : SYSUTIL
* `cpl::util::log`       : SYSUTIL
* `cpl::util::network`   : NETWORK

* `cpl::detail_`         : Private namespace for implementation details.  Do
  not use those in client code.

## Error reporting

  Errors are reported as exceptions derived from `std::exception` with a 
meaningful `what()` message.


## Supported platforms

  Cpp-lib is written in ISO C++ and makes use of C++11 features.
Due to multiple problems with GNU g++, we currently recommend clang++
(October 2015).

- clang++: Based on LLVM 3.5.0 or higher (MacOS X and Linux).
- The Microsoft Visual Studio Express (C++) and CYGWIN builds are looking
  for maintainers.

It is generally possible to use subsets of cpp-lib independently, e.g. 
for embedded applications.


## Building cpp-lib

### Dependencies

- A C++ compiler supporting C++11 or later (Tested: clang++)
- GNU make (BSD make does *not* work)
- The [BOOST] [1] header files.
- The [Eigen] [10] library
- If `PNG_STUFF` is enabled: [libpng] [8] and [png++] [9]

__Boost, png++ and Eigen are used in a header-only way.__

- Installing dependencies on Ubuntu:
  * `sudo apt-get install libeigen3-dev`
  * `sudo apt-get install libpng++-dev`

- Installing clang++ on Ubuntu (example: Version 3.6):
  * `sudo apt-get install clang-3.6`
  * `sudo apt-get install libc++-dev`
  * `sudo apt-get install libc++abi-dev`

- Installing g++-5 on old Ubuntu versions (e.g. 14.04):
  `sudo add-apt-repository ppa:ubuntu-toolchain-r/test`
  `sudo apt-get update`
  `sudo apt-get install g++-5`

### Tested versions

- Boost: 1.58.0, 1.60.0 (`#include "boost/version.hpp"`)
- libpng: 1.2.50, 1.6.18 (`libpng-config --version`)
- png++: 0.2.5
- Eigen: 3.2.5

### Build

- `cd prj`

- Create symbolic links appropriate for your system or copy the files:


#### Setup on Ubuntu

  ```
  ln -s def.compiler.clang-3.6 def.compiler
  ln -s def.platform.linux-clang-libc++ def.platform
  ```

#### Setup on MacOS X (Darwin)

  ```
  ln -s def.compiler.clang def.compiler
  ln -s def.platform.darwin def.platform
  ```

#### Compile

- `make -j4 tests` to build the library and tests.

-  Use `export debug=true; make; make tests` to build a debug version of 
  the library and the tests.
  Release and debug versions are kept in separate directories.

  __Always use the same `debug` setting (true or false) for `make`,
  `make tests` and `make clean`.__

#### Test

  `cd testing/`

  `./run-all-tests.sh`

  Test output is written to `testing/data/golden-output`.  Run
  `git diff` to look out for unexpected changes.


#### Clean
 Use `make clean` to delete object files and executables.

  __The supplied Makefile assumes GNU make.__

  The Makefile is based on separate configuration of compiler (file
def.compiler), platform (def.platform) and include files (def.includes).

  Examples are provided for certain supported platforms and compilers
(including cross compilation with MinGW).  See `def.platform.*` and
`def.compiler.*`.  Not all combinations make sense!

  The library and tests will be built under `.../cpp-lib/build/$(PLATFORM)` .
Note that many tests use input files, which are found in
`.../cpp-lib/testing`.

  The `def.platform.*` files set the variables `WINDOWS_STUFF`, 
`POSIX_STUFF` and `POSIX1B_STUFF` to yes to include the respective 
platform-specific functionality.  Exactly one of `WINDOWS_STUFF` or 
`POSIX_STUFF` must be set to yes.

  `WINDOWS_STUFF` indicates that cpp-lib should use the Microsoft Windows
API, `POSIX_STUFF` the POSIX (SUSv3) API.  Note that the target
operating system may be Windows while the POSIX API is used, as is the
case for the cygwin environment.

  Depending on the platform settings, the Makefile will create symbolic
links (or copies for compilers which don't support symbolic links) of
the respective platform subdirectories in the source and header
directories to `platform` and `platform_rt`, resp.


###  BOOST notes

Set `BOOST_INCLUDES` in def.includes to point to your BOOST header files.

  If you use a cross compiler, it may be preferable to have
  a separate set of BOOST header files (i.e., not under /usr/include).



### Windows compilation notes

  GNU make is available e.g. in [cygwin] [2].

  No project for the Visual Studio .NET IDE is supplied.  To compile
using the Microsoft C++ compiler, start the Visual Studio .NET command
prompt and cygwin from it (typically, enter c:\cygwin\cygwin).  This
procedure ensures that the environment variables for the compiler are
visible inside the cygwin environment.

  Code using the Windows API (`WINDOWS_STUFF` set to yes) assumes that the
WIN32 preprocessor symbol is set.


### FreeBSD compilation notes

  The default make utility of FreeBSD is *not* GNU make.  You need to
install and use the gmake port.


## Using cpp-lib in your own code

  Headers for client code are in include/cpp-lib and include/cpp-lib/sys.

  Use `#include "cpp-lib/xyz.h"` in your code and set the include path
to `cpp-lib/include` in your project.

  Do not include anything from the the platform-specific
subdirectories of include/cpp-lib (currently, posix/, posix1b/ and windows/).

It is possible to use just parts of cpp-lib.  To do so, add the
required individual cpp-lib sources and headers to your project.
This way you can achieve fine-grained control over the dependencies.  
E.g., it's possible to use MATH, UTIL and CRYPT without BOOST.


### Debug builds

  Many functions in cpp-lib are rather short inline functions.  It may make
debugging easier if inlining is switched off for debug builds.  On g++, use
-fno-inline.


## Directory structure

* `src/`              C++ sources (.cpp).
* `include/cpp-lib/`  C++ headers (.h).
* `testing/`          Test/example programs.
* `testing/data/`     Test data and golden output.
* `prj/`              Makefiles and platform/compiler definition files.
* `obj/`              Build directory, created by the makefile.
* `obj/{opt,dbg}`     Object files and libraries.
* `bin/{opt,dbg}`     Executable files


## Caveats

  Some mathematical algorithms, notably the minimize() function, depend on the
availability of floating point representations of infinity and not-a-number.
Most modern platforms satisfy this requirement.


  __The `realtime_scheduler` class may only be instantiated once per process
on POSIX systems.__

## Bugs

Please see TODO.md.


## Coding style

- General: Stick with the style of code around you.

- Lines no longer than 80 characters.

- Two characters indentation.

- No tabs, use spaces instead.

- UNIX line endings.

- C++ style comments (//).

- `lower_case_and_underscore` style.


### Library design principles

- Performance and ease of use.

- Realtime guarantee:  Generally, data structures and algorithms are designed 
  to allocate all resources at initialization time.

- Seamless integration with the C++ standard library.  E.g., the TCP and
  serial interface provide standard C++ streambufs.

- RAII (resource acquisition is initialization).  Constructors
  either yield a ready-to-use object or throw.

- Better safe than sorry.  Assertions even in tight loops.

- Platform independency, not multiplatform dependency.  Avoid
  `#ifdef _LINUX_/WIN32/etc.` mess, but rather factor out minimal generic 
  interfaces for platform-specific functionality (see e.g. posix/wrappers.cpp
  and windows/wrappers.cpp)

- Orthogonality/DRY: Don't repeat yourself


[1]: http://www.boost.org/
[2]: http://www.cygwin.com/
[3]: http://jmarshall.com/easy/http/ "HTTP made really easy."
[4]: http://www.kyb.tuebingen.mpg.de/bs/people/carl/code/minimize/
[5]: http://en.wikipedia.org/wiki/Nelder-Mead_method
[6]: http://de.wikipedia.org/wiki/Downhill-Simplex-Verfahren
[7]: http://reference.wolfram.com/mathematica/tutorial/ConstrainedOptimizationGlobalNumerical.html "Mathematica Tutorial: Numerical Nonlinear Global Optimization."
[8]: http://www.libpng.org/
[9]: http://www.nongnu.org/pngpp/
[10]: http://eigen.tuxfamily.org/


## See also

  See README-networking.md for information on the TCP/IP and UDP classes.


## Copyright

  Cpp-lib is copyright (C) 2014 and onwards, KISS Technologies GmbH and its
contributors.  Please give due credit when using it.  Please see the file
`LICENSE-2.0.txt` for details.

  OpenAIP data is owned by Butterfly Avionics GmbH and licensed under the CC BY-NC-SA.
Cpp-lib uses a small amount of openAIP data for testing purposes.
