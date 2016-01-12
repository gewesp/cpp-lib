# BUGS

## Windows specific

- Windows apparently doesn't return file modification times correctly
  on local hard drives (GetFileTimes()).  Thus the existance of
  `file_name_watcher`.

- Be careful not to forget the respective compiler flags when using
  threads!  Consult your compiler's documentation as to which parts of
  C++ are implemented thread-safely and which aren't.


# Future directions / TODO

- Documentation:  Extract the comments from the header files and put them in a
  printable document.

- Streamline the testing system and add a script that automatically runs the
  unittests.

- Use more C++11/C++14 features as they become available, e.g.
  `<random>`

- Make sure that floating point random numbers from std::random are 
  deterministic *across platforms* (they're used a lot in tests!).  Better yet, 
  make test outcomes independent of concrete random number sequence.

- Refactor `convert()/check_...` etc. in registry.h.  Too much overloading
  is going on.  Name `T const& convert( any& )` `extract`.  Name the template
  `convert_template()`.

- Factor out crypt streambufs?  Stateful reader/writer classes
  generalizing `streambuf_traits`?

- Replace symlinks by different include paths?

- Revamp error handling: general `throw_os_exception()` function based on
  last error or `strerror_r()`, resp.  Get rid of `STRERROR_CHECK()`.

- Consolidate realtime stuff:  Multiple schedulers should be possible
  with POSIX1B (cf. `timer_create()`, 2.4, etc. in SUSv3).  itimer-based:
  should be singleton.  itimer-based:  SIGARLM will be blocked only for
  current thread, therefore have to instantiate it before any threads
  are created!  Check blocking semantics for realtime signals (maybe
  they're blocked by default?).

- Implement serial interface classes on systems other than Windows.

- Write `<cctype>` overloads for signed/unsigned char.  Used in util.h and
  registry.cpp.  Alternatively, document ASCII only.

- Add an AES implementation.

- Templatize `table<>` on index type?

- Logger: Base on varlist (currently, code is duplicated).

## Networking specific

- Implement performance preferences?

  See Java implementation:
    http://java.sun.com/j2se/1.5.0/docs/api/allclasses-frame.html

- Avoid/ignore SIGPIPE on socket write on Windows and other systems
  (it is implemented on Linux and Darwin/FreeBSD).  Unfortunately,
  there doesn't seem to be a portable (POSIX) way to do that.

- Optimize UDP implementation:  Avoid buffer copies in case of
  contiguous ranges.

- Check defaults for address reuse and broadcasting in all constructors.

- `shutdown()`:  Remove unnecessary shutdowns (e.g. if other side already
  has shut down).  Try to do this without a stateful socket
reader/writer class, if at all possible (thread safety!).

- Implement `iowait()` (multiplexing).

- Implement the exception hierarchy from N1925.

- Put it into `std::tr2`.

- More example code.


# Feature requests

- Lexer: Support for different line ending styles recognized on the fly?

- New `registry::read_from()` overloads with descriptive parameters (throw
  on redefinition...).  Integrate the flag with grammar?

- Registry: extend to expression parsing (similar to Mathematica parser).
