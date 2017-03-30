# mart-common

This is a small utilities library for usage in software related to the mart project at the chair for Real-Time Computer Systems (RCS) at TU-Munich, developed by Michael Balszun.

You can find the most recent version on the lrz gitlab server at https://gitlab.lrz.de/rcs_mart/mart-common.git. 

The library is in an early (alpha) phase and currently, no versioning scheme is used. Compatibility may be broken between each and every commit. That being said, except for types in the subfolder "experimental", we try to keep the interfaces as stable as possible. 

### Structure
Currently this is a header-only library. For usage either add "./include" to your include path or add it as a module in cmake

### Content
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
  Can be used as a parameter for many functions that ordinarily would e.g. take a `vector<T>` but is more generic than that 
 
- `ConstString.h`:   
  String that can't be modified after creation using a ref-counted implementation. 
  Main advantages over std::string (when applicable): 
	- Doesn't allocate memory when constructed from string litteral
	- Concatenation of multiple string-like objects requires only a single dynamic memory allocation

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
 
 ####### Folders #######
 The following folders are inside include/mart-common:
  
- `cpp_std`:  
  Contains c\++14/c++17 types that are missing in the standard library shipped with gcc 4.8 / 4.9 which we have to use at the moment. They are designed to be an exact drop in replaccement for the standard types and once the compiler is upgraded they can directly be replaced

- `experimental`:  
  Contains new features, classes and functions, whose interface might change at any point in the future

- `logging`: Subfolder for classes related to logging. If you jsut want to use mar tlog, simply include MartLog.h from the main include directoy
  
### Contributing
  
  For bugfixes, feature or improvement requests, please contact michael.balszun@tum.de or raise an issue at the gitlab repository at https://gitlab.lrz.de/rcs_mart/mart-common (you might need to get the access rights first)
