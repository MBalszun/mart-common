# mart-common [![Build Status](https://travis-ci.org/tum-ei-rcs/mart-common.svg?branch=master)](https://travis-ci.org/tum-ei-rcs/mart-common) <!-- [![codecov](https://codecov.io/gh/tum-ei-rcs/mart-common/branch/master/graph/badge.svg)](https://codecov.io/gh/tum-ei-rcs/mart-common) -->

This is a small utilities library for usage in software related to the mart project at the chair for Real-Time Computer Systems (RCS) at TU-Munich, developed by Michael Balszun.

The library is in an early phase and currently, no versioning scheme is used. Compatibility may be broken between each and every commit. That being said, except for types in the subfolder "experimental", we try to keep the interfaces as stable as possible.

We currently require c++17 and cmake 3.13. 

# Structure
There are three components:
- im_str: This is a separate library (https://github.com/MBalszun/im_str) providing an immutable string class which is integrated here for convenience sake (usable as Mart::im_str target in cmake)
- mart-common: A header only collection of various utility functions and sub-libraries (Mart::common)
- mart-netlib: A simple portability layer over the raw c-bsd socket interface (Mart::netlib). This library is NOT header only!


# Content
This is a short (and probably out of data) overview over the content provided by this library


- `MartLog.h`: Logging framework 



- `ArrayView.h`:
  Contains an array view class similar to `gsl::span`. Basically a wrapper around a pointer and a length.
  Can be used as a parameter for many functions that ordinarily would e.g. take a `vector<T>&` but is more generic than that

- `ConstString.h`:  **By now essentially a wrapper around im_str / im_zstr:**
  String that can't be modified after creation using a ref-counted implementation.
  Main advantages over std::string:
	- Doesn't allocate memory when constructed from string litteral
	- Concatenation of multiple string-like objects requires only a single dynamic memory allocation
    - Copying and creating substrings is pretty cheap

- `exceptions`: Common exceptions used in this library. In particular making use of im_str

- `MartTime.h`:
  Collection of timing related function. Main purpose is making the usage of std::chrono facilities less painfull
  (e.g. when checking for timeouts or interacting with legacy api's)

- `MartVec.h`: A very simple vector class, that provides support for basic vector operations like addition, (matrix-) multiplication, rotation 

- `PrintWrappers.h`:
  Wrapper classes for some types (currently only std::chrono::duration) that determine how those types are formatted when inserted into a stream
  `std::cout << sformat(std::chrono::milliseconds(10))  << '\n'; // prints "10ms"`

- `ranges.h`:
  Some utility range classes (mainly for usage with range based for)
  ` for (auto i : irange(0,20)){} // i will have values from 0 to 19 inclusive`

- `random.h`: Producing a pseudo random numer with a single function call.

- `StringView.h`:
  Class similar (but not compatible) to std::string_view. Essentiallly a targeted ArrayView, that encapsulates a pointer and a length.
  
- `TypePunningUnion`: Class that allows reinterpretation of one bitpattern as a different bitpattern in a c++ conforming manner. This is primarily used to allow you to read bytes from a stream and then cast theminto the c++ type.

- `utils.h`:
  Random collection of small helper functions and classes

## Folders
 The following folders are inside include/mart-common:
 
 - `algorithms`:
  Wrapper around some standard algorithms that take a range (anything with a begin and end) as a parameter instead of two iterators.
  ```
   vector<int> v{...};
   sort(v); // instead of sort(v.begin(),v.end());
  ```
  
- `enum`: Various helpers with the goal to define sets of enums which can be easily converted to and from stirngs and iterated over.

- `cpp_std`:
  Brings some modern c++14/17/20 etc. types into the mart namespace - either as an alias or as a reimplementation. Mainly leftover from the time when we had to use the standard library that shipped with gcc 4.8 / 4.9. They are designed to be an exact drop in replaccement for the standard types and once the compiler is upgraded they can directly be replaced

- `experimental`:
  Contains new features, classes and functions, whose interface might change at any point in the future. Noticable this currently (2017-06-07) e.g. contains:
   - `CopyableAtomic`: Atomics that can used as member objects without disabeling the copy and move operations
   - `DevTools`: A few utility classes that can be helpfull when evaluating a certain design.
   - `DynLimArray.h` (Name subject to change in the near future). A stack allocated array with compiletime capacity but a size that is determined at runtime during construction. Ideally suited to return e.g. a short array of numbers from a function
   - `Optional.h`: A class similar to std::optional
   - `out_param.h`: Class to make outparameter explicit at call site
   - `network`: Folder containing slim abstraction layer above (win/unix) sockets for compatibility across windows and linux udp and tcp sockets (superseeded by netlib)

- `logging`: Subfolder for classes related to logging. If you just want to use martlog, simply include MartLog.h from the main include directoy

- `mt`: Datastructures related to multithreading (e.g. a tripplebuffer or queues)

# Contributing

  For bugfixes, feature or improvement requests, please contact michael.balszun@tum.de or raise an issue at the gitlab repository at https://github.com/tum-ei-rcs/mart-common

# Tests

If you want to boild the tests, run cmake with the `-DMART_COMMON_INCLUDE_TESTS=ON` option and call `ctest .`
Testcoverage is nowhere as complete as we would like and we rely far too much on the tests of programs that use the components in this library. 
