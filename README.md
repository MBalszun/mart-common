# mart-common [![Build Status](https://travis-ci.org/tum-ei-rcs/mart-common.svg?branch=master)](https://travis-ci.org/tum-ei-rcs/mart-common) [![codecov](https://codecov.io/gh/tum-ei-rcs/mart-common/branch/master/graph/badge.svg)](https://codecov.io/gh/tum-ei-rcs/mart-common)

This is a small utilities library for usage in software related to the mart project at the chair for Real-Time Computer Systems (RCS) at TU-Munich, developed by Michael Balszun.

The library is in an early phase and currently, no versioning scheme is used. Compatibility may be broken between each and every commit. That being said, except for types in the subfolder "experimental", we try to keep the interfaces as stable as possible.

We currently require c++17 and cmake 3.10. An older version based on c++11 & cmake 2.8.12 can be found in the branch `cpp11`. From time to time we cherry-pick some commits from master to that branch if they don't rely on modern toolchains

# Structure
Currently this is a header-only library with all header files lying in the `./include/mart-common` sub folder. For usage either add "./include" to your include path or add the root folder as a subfolder in cmake

# Content
This is a short (and probably out of data) overview over the content provided by this library

- `algorithm.h`:
  Wrapper around some standard algorithms that take a range (anything with a begin and end) as a parameter instead of two iterators.
  ```
   vector<int> v{...};
   sort(v); // instead of sort(v.begin(),v.end());
  ```
  Currently (2016-11-08) only a few example algorithms are provided but expect more to come

- `ArrayView.h`:
  Contains an array view class similar to `gsl::span`. Basically a wrapper around a pointer and a length.
  Can be used as a parameter for many functions that ordinarily would e.g. take a `vector<T>&` but is more generic than that

- `ConstString.h`:
  String that can't be modified after creation using a ref-counted implementation.
  Main advantages over std::string:
	- Doesn't allocate memory when constructed from string litteral
	- Concatenation of multiple string-like objects requires only a single dynamic memory allocation
    - Copying and creating substrings is pretty cheap

- `MartLog.h`:
  Header that provides a simple logging mechanism

- `MartTime.h`:
  Collection of timing related function. Main purpose is making the usage of std::chrono facilities less painfull
  (e.g. when checking for timeouts or interacting with legacy api's)


- `PrintWrappers.h`:
  Wrapper classes for some types (currently only std::chrono::duration) that determine how those types are formatted when inserted into a stream
  `std::cout << sformat(std::chrono::milliseconds(10))  << '\n'; // prints "10ms"`

- `ranges.h`:
  Some utility range classes (mainly for usage with range based for)
  ` for (auto i : irange(0,20)){} // i will have values from 0 to 19 inclusive`

- `StringView.h`:
  Class similar (but not compatible) to std::string_view. Essentiallly a targeted ArrayView, that encapsulates a pointer and a length.

- `utils.h`:
  Random collection of small helper functions and classes

## Folders
 The following folders are inside include/mart-common:

- `cpp_std`:
  Brings some modern c++14/17/20 etc. types into the mart namespace - either as an alias or as a reimplementation. Mainly leftover from the time when we had to use the standard library that shipped with gcc 4.8 / 4.9. They are designed to be an exact drop in replaccement for the standard types and once the compiler is upgraded they can directly be replaced

- `experimental`:
  Contains new features, classes and functions, whose interface might change at any point in the future. Noticable this currently (2017-06-07) e.g. contains:
   - `CopyableAtomic`: Atomics that can used as member objects without disabeling the copy and move operations
   - `DevTools`: A few utility classes that can be helpfull when evaluating a certain design.
   - `DynLimArray.h` (Name subject to change in the near future). A stack allocated array with compiletime capacity but a size that is determined at runtime during construction. Ideally suited to return e.g. a short array of numbers from a function
   - `Optional.h`: A class similar to std::optional
   - `out_param.h`: Class to make outparameter explicit at call site
   - `network`: Folder containing slim abstraction layer above (win/unix) sockets for compatibility across windows and linux udp and tcp sockets

- `logging`: Subfolder for classes related to logging. If you jsut want to use martlog, simply include MartLog.h from the main include directoy

# Contributing

  For bugfixes, feature or improvement requests, please contact michael.balszun@tum.de or raise an issue at the gitlab repository at https://github.com/tum-ei-rcs/mart-common

# Tests
**TBD**
The repository currently only provides a unit test skeleton. What unit tests exist (we are far from full coverage) are part of a different application using this library.
We are still considering how to best ship them while minimizing impact on projects that just want to include the library as a git submodule.
