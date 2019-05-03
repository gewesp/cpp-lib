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

- Open Glider Network ([APRS](doc/ogn-aprs.md) and DDB) client
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
- Undulation conversion between WGS84 and MSL altitudes (Earth Geoid Model,
  function `geoid_height()`).
- A geographic database with spatial search and openAIP input
  (suitable for airport reference points).

### HTTP

- A very simple HTTP client

### MAP

- Web Mercator tile mapping


### MATH

- Some components are based on the [Eigen] matrix library.
- 3-dimensional geometry, quaternions, Euler angles, etc.
- ODE solvers and some state-space modeling blocks.
- Unconstrained multidimensional minimization with gradients (a C++ version
  of [minimize]).
- Unconstrained multidimensional minimization without gradients (see
  [Nelder/Mead], [downhill simplex], [Wolfram optimization tutorial]).
- A spatial index based on boost::rtree.


### NETWORK

- Easy TCP/IP and UDP connections.  Iostreams implementation for TCP 
  streams.  See [README-networking](README-networking.md) for more information.
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

  Cpp-lib is written in ISO C++ and makes use of C++14 features.
We recommend clang++ to compile cpp-lib.

- clang++: Based on LLVM 3.5.0 or higher (Tested: Linux).
- The Microsoft Visual Studio Express (C++) and CYGWIN builds are looking
  for maintainers.

It is generally possible to use subsets of cpp-lib independently, e.g. 
for embedded applications.


## Building cpp-lib

### Dependencies

- A C++ compiler supporting C++14 or later (Recommended: clang++)
- [CMake], version 3.1 or later
- The [BOOST] libraries, version 1.58.0 or later
- The [Eigen] library
- The [libpng] and [libpng++] libraries

See Tested Versions below.

__Boost, png++ and Eigen are currently used in a header-only way.__

- Installing dependencies on Ubuntu:
  * `sudo apt install libeigen3-dev`
  * `sudo apt install libpng++-dev`
  * `sudo apt install libboost1.65-dev`

- Installing clang++ on Ubuntu:
  * `sudo apt install clang-7`
  * `sudo apt install libc++-7-dev`
  * `sudo apt install libc++abi-7-dev`

### Tested versions

- Boost: 1.58.0, 1.60.0 1.65.1(`#include "boost/version.hpp"`)
- libpng: 1.2.50, 1.6.18, 1.6.34 (`libpng-config --version`)
- png++: 0.2.5, 0.2.9 (`dpkg --status libpng++-dev`)
- Eigen: 3.2.5, 3.3.4-4 (`dpkg --status libeigen3-dev`)

### Tested hardware platforms

See `uname -m`

* `aarch64` (ARMv8)
* `x86_64` (Intel)

### Build

From the main directory, run `scripts/build.sh`.

Please see the script for options (build type, compiler etc.).


### Test

Prerequisite: Successful build.

  `cd testing/`

  `./run-all-tests.sh`

  Test output is written to `testing/data/golden-output`.  Run
  `git diff` to look out for unexpected changes.


### Windows compilation notes

  The Windows build is looking for a maintainer.  The first TODO item
would be to add CMake support.  It should be possible 
to compile using Visual Studio, cygwin or MinGW.


### FreeBSD compilation notes

  The build should work, but installation of requirements may be different
to Ubuntu.  Maintainers welcome.

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
* `prj/`              __DEPRECATED__, use cmake instead.  Makefiles and platform/compiler definition files.
* `obj/`              Build directory, created by the makefile.
* `obj/{opt,dbg}`     Object files and libraries.
* `bin/{opt,dbg}`     Executable files


## Caveats

  Some mathematical algorithms, notably the minimize() function, depend on the
availability of floating point representations of infinity and not-a-number.
Most modern platforms satisfy this requirement.


  __The `realtime_scheduler` class may only be instantiated once per process
on POSIX systems.__

## Bugs and TODOs

* Update CI for cmake and remove make-based build infrastructure

Please see also [TODO](TODO.md) and issues on the github page.


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


## Contributor guidelines

- Please follow the library design guidelines above.
- Commits that add or change functionality should be kept small and
  with well readable diffs for code review.
- To achieve the above, isolate as much as possible of the work in 
  'no functional change' (NFC) commits.  That is, refactor before
  you add or change functionality.
- Any whitespace changes should be in a separate NFC commit.
- Make sure that NFC commits don't change any test results or break tests.
- Add test cases for new functionality, modify test cases for modified
  functionality.

### Commit messages

Examples:
- ADD: Shiny new feature
- MOD/NFC: Refactoring the xyz component (zero functional change)
- REMOVE: We no longer need the abc() function (use xyz() instead)
- FIX: Fixed a bug in the uvw module causing the server to crash.

## References

[BOOST]: http://www.boost.org/
[Cygwin]: http://www.cygwin.com/
[minimize]: http://www.kyb.tuebingen.mpg.de/bs/people/carl/code/minimize/
[Nelder/Mead]: http://en.wikipedia.org/wiki/Nelder-Mead_method
[downhill simplex]: http://de.wikipedia.org/wiki/Downhill-Simplex-Verfahren
[Wolfram optimization tutorial]: http://reference.wolfram.com/mathematica/tutorial/ConstrainedOptimizationGlobalNumerical.html "Mathematica Tutorial: Numerical Nonlinear Global Optimization."
[libpng]: http://www.libpng.org/
[libpng++]: http://www.nongnu.org/pngpp/
[Eigen]: http://eigen.tuxfamily.org/
[CMake]: https://cmake.org/


## See also

  See [README-networking](README-networking.md) for information on the TCP/IP and UDP classes.


## Copyright

  Cpp-lib is copyright (C) 2014 and onwards, KISS Technologies GmbH and its
contributors.  Please give due credit when using it.  Please see the file
`LICENSE-2.0.txt` for details.

  OpenAIP data is owned by Butterfly Avionics GmbH and licensed under the CC BY-NC-SA.
Cpp-lib uses a small amount of openAIP data for testing purposes.
