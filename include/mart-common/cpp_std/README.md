# cpp_std

## Content:

This folder contains c++11 implementations of standard library elements that have been introduced by
- later c++ standards
- technical specifications
- c\++11 but are not yet supplied by our toolchains (g++4.8)

In the future, some of the types might become typdefs for the standard library supplied versions and might eventually be removed

### Notice:
- some code snippets might originate from https://cppreference.com

## Design rules:

- Each member introduced here is supposed to be a drop-in for the standard library type. Types that only provide "similar" functionality but are incompatible
  are not placed into this folder
- Each header file is named after the corresponding standard library header file that would implement the according type / functions
- Each header file includes at least the standard library header file with the same name (if applicable) (e.g. "tuple.h" includes ```<tuple>```)
- No headerfile will include any file that is not either a standard library header file or inside this folder
- Each type lives in the namespace `mart` (or subnamespace)


