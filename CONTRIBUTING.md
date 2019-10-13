# Contributor guidelines

Thanks for contributing to `cpp-lib`.  To ensure we're all on the same
page, please follow the guidelines and principles below.

## Coding style

* Most important: Stick with the style of code around you.
* Base: Google code style for C++, with the following changes:
  * Exceptions are not demonised, but encouraged.
  * `snake_case` style.
  * Use `const` by default, everywhere where it's possible.  Period.
    Notice:  This probably means that your main function should look
    like this:
```
int main(const int argc, const char* const* const argv)
```
* Lines no longer than 80 characters.
* No tabs, use spaces instead.
* UNIX line endings.
* C++ style comments (//).


## Library design principles

* Performance and ease of use.
* Realtime guarantee:  Generally, data structures and algorithms are designed 
  to allocate all resources at initialization time.
* Seamless integration with the C++ standard library.  E.g., the TCP and
  serial interface provide standard C++ streambufs.
* RAII (resource acquisition is initialization).  Constructors
  either yield a ready-to-use object or throw.
* Better safe than sorry.  Assertions even in tight loops.
* Platform independency, not multiplatform dependency.  Avoid
  `#ifdef _LINUX_/WIN32/etc.` mess, but rather factor out minimal generic 
  interfaces for platform-specific functionality (see e.g. posix/wrappers.cpp
  and windows/wrappers.cpp)
* Orthogonality/DRY: Don't repeat yourself
* Add test cases for new functionality, modify test cases for modified
  functionality.



## Git usage

* No branches unless absolutely necessary (check with maintainer; consider
  using feature toggles etc.).
* Make baby steps.  Commits of even 1 or 2 lines are OK.  It's OK to
  commit incomplete work or even stubs, if clearly marked as such
  in the code.
* `ADD`, `MOD`, `FIX` etc. should go into separate commits whereever
  possible.
* `NFC` commits should always be isolated.
* The above applies especially to whitespace only changes.
* Don't break the build.


### Commit messages

Commit messages **MUST** begin with one of `ADD:`, `MOD:`, `FIX:`,
optionally qualified with `NFC`.

`NFC` must be used if the commit causes _No Functional Change_.

Examples:
* ADD: Shiny new feature
* MOD/NFC: Refactoring the xyz component (zero functional change)
* ADD/NFC: Stub for function `abc()`
* FIX: Fixed a bug in the `uvw` module causing the server to crash.
