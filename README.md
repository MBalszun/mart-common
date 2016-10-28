### mart-common
This is a small utilities library for usage in software related to the mart project at rcs


### Content

- ArrayView.h:
  Contains an array view class similar to gsl::span. Basically a wrapper around a pointer and a length 
 
- ConstString:
  String that can't be modified after creation using a ref-counted implementation. 
  Main advantages over std::string (when applicable): 
	- Doesn't allocate memory when constructed from string litteral
	- Concatenation of multiple string-like object requires only a single dynamic memory allocation
	
- MartTime.h:
  Collection of timing related function. Main purpose is making the usage of std::chrono facilities less painfull
  (e.g. when checking for timeouts or interacting with legacy api's)

- utils.h:
  Random collection of small helper functions and classes
  
- cpp_std
  Contains c++14/c++17 types that are missing in the standard library shipped with gcc 4.8 / 4.9 which we have to use at the moment. They are designed to be an exact drop in replaccement for the standard types and once the compiler is upgraded they can directly be replaced

- experimental
  Contains new feature, classes, functions, whose interface might change at any point in the future
  
For bugfixes, feature or improvement requests, please contact michael.balszun@tum.de  


### Contributing

This library is an ongoing effort and might (will) contain bugs and lack features. If you want to help with the deveolpment and improvement, please contact michael.balszun@tum.de and/or provide contribute to https://gitlab.lrz.de/rcs_mart/mart-common 

