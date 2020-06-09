# ImStr [![Build Status](https://travis-ci.org/MBalszun/im_str.svg?branch=master)](https://travis-ci.org/Mike-Bal/im_str)
An immutable string class which doesn't allocate if constructed from a string litteral.

**NOTE: This readme is an early draft. It’s known to be incomplet and incorrekt**

# Desciption
The central class of this library is the immutable string type `im_str`. Essentially it is a `std::string_view` (with some additional member functions like `split`) that either points to a string litteral or owns the data it points to (in a shared manner).

The main idea behind it is to have a light weight string type that minimizes allocations - in particular when created when from string litterals - and is easier to analyze for the optimizer than `std::string`.

# Usage

The library is header only so you can just copy the content of the `include` directory to whereever you want it and add the folder to the include path. The other possiblility is to use it as a subdirectory from your cmake file i.e.

    add_subdirectory( <path-to-im_str-root> )
    target_link_libraries( my_app PRIVATE ImStr::im_str )

This library requires c++17 (the cmake target deliberately doesn't pass the cxx_std_17 requirement though, because I don't want it to implicitly change the c++ stnadard with which you compile your project)

# API
## Overview
At this point, the API is essentially a superset of `std::string_view` with some added member functions for string splitting. 
The advantage compared to `std::string_view` is that `im_str` ensures the string data remains valid **and unchanged** during its lifetime, so there is no danger of dangling (e.g. when creating a subview of a temporary string).

Usage is pretty much the same as you would expect from an immutable, ref-counted string type (TODO: Add full API ref)

    #include <im_str/im_str.hpp>

    #include <iostream>
    #include <string>

    int main() {
		using namespace mba;
        
		im_str name = "John";
		assert( name == "John" );
		assert( name.size() == 4 );
		std::cout << name; // Will print "John";

		im_str cpy = name;
		name       = im_str( "Jane Doe" );
		assert( cpy == "John" ); //cpy hasn't changed when reassigning the original name variable

		auto [first, second] = name.split_on_first( ' ' );

		name = im_str {};
		cpy  = im_str {};

		assert( first == "Jane" );
		assert( second == "Doe" );
    }


Even though `im_str` doesn't implement SSO (yet), there isn't a single allocation happening in the above code. 
Allocations are only neccesary, when an `im_str` is created from something other than a string litteral or another `im_str`:

    std::string name = "Mike";
    mba::im_str  is  = mba::im_str( name );          // This allocates

    mba::im_str full_greeting = mba::concat( "Hello, ", name, "!\n" ); // This will also allocate (once)

    std::cout << full_greeting; // Prints "Hello, Mike!", followed by a newline

Just like string_view, im_str is not guaranteed to be zero terminated (this can be queried via the member `im_str::is_zero_terminated()` though).
However, the library also provides the type `im_zstr`, which is from `im_str` and is guaranteed to be always zero terminated. `concat` actually returns a `im_zstr`

	using namespace mba;
    
	im_str full = "Hello, World!";
	assert( full.is_zero_terminated() );

	im_str sub = full.substr( 0, 3 );
	assert( sub.is_zero_terminated() == false );

	im_zstr subz = sub.create_zstr();    // This will allocate because `sub` isn't zero terminated
	assert( subz.is_zero_terminated() ); // This will always be true

	im_zstr fullz = std::move( full ).create_zstr(); // This  will not allocate and not change the ref count
	assert( full.empty() );
	c_func( fullz.c_str() );
    

## [TODO] Full API description
### `im_str`
### `im_zstr`

Same as `im_str`, but guaranteed to be zero terminated and hence provides `.c_str()` member. This is e.g. the result from `concat`.
When created from a `im_str` via `create_zstr()`, the member function first checks if the string data is already zero terminated, in which case it just bumps the ref-count and the created `im_zstr` object shares the original data. Otherwise creates a copy of the data on the heap.

### `concat`

Free function that takes an arbitrary number of arguments that can be converted to `std::string_view` and concatenates them into a `im_zstr`. This allways allocates.


# Why another string class?

**Disclaimer**: Essentially, this is a simplified, c++17 version of a type that has mainly been used in (semi-) embedded system projects and was developed at a time where the toolchains for those systems either didn't provide a `std::string` at all, or at least not a SSO version. While it can also be usefull in desktop and server applications, the drawback of using a non-standard string type with less micro-optimization compared to modern implementations will likely outweight the benefits.

Two important optimizations compared to `std::string` are that 1) the internal data is ref-counted, so copying is (comparibly) cheap and 2) when constructed from a string litteral (more precisely, when constructed from a const char array) it doesn't allocate at all, but just points to the string litteral. Both of those optimizations are possible because other than `std::string`, `im_str` **doesn't allow mutation of the underlying data**.

In particular the creation from string litterals is a) very efficient and can be performed at compile time and is b) largely transparent to the optimizer. This generally leads to much better/smaller code generation.

<!--
Of course a simple `const char*` or `std::string_view` that point to a string litteral have the same properties, but in those cases, the types don't guarantee that the data stays valid and unchanged during the lifetime of the variables. And you effectively encode the assumptions that they point to some static data into your API. This can become especially problematic when used as member variables of an object


E.g.:

    struct Config {
        im_zstr local_name;
        im_zstr remote_name;
    };
    const Config config {"foo", "bar"};

doesn't require allocations, the values of `config.local_name` and `config.remote_name` are generally well known to the compiler and used during optimizations and

Also, in code like

    auto[first_name, last_name] = full_name.split(' ');

first_name and last_name

Where the code operating it would need to know, if it has to deallocate the variables or not and

    struct Config {
        std::string local_name;
        std::string remote_name;
    };

where compilers usually are not able to optimize other code based on the value of `config`.

-->

# Implementation

TODO

# Controversial Design Decisions
Generally speaking, this library tends to err on the side of simplicity over genericity / flexibility. It is mainly used in a relativley narrow set of applications and I rather optimize for specific usecases than paying the overhead (bet it compile-time, runtime, binary size or code complexity) which often results from overly generic designs whose benefits I'm not (yet) able to reap. Also, the lib is in it's infancy, so I just didn't come around to properly implement some interesting features.

## Inheriting from `std::string_vew`

`im_str` inherits publicly from `std::string_view` and `im_zstr` inherits publicly from `im_str`.
Many will argue that public implementation inheritance from a standard library class is about as bad a an idea as it gets.

However, the simple fact is that reusing `std::string_view` eliminates a whole lot of boiler plate code and as we all know, the most efficent and most bugfree code is the code you don't have to write. More importantly, most of the drawbacks don't really apply in this case:

- Deletion via pointer to base class:
  `std::string_view` doesn't have a virtual destructor, so if you have - lets say - a `std::unique_ptr<std::string_view>` that actually points to a `im_str` the wrong destructor will get called and you'll have UB. This is true but I just can't think of any valid reason to have a `std::unique_ptr<std::string_view>` outside of generic code in the first place. And even within generic code, it is very hard for me to imagine a situation, where such a pointer would actually end up pointing at a `im_str`.

- Object slicing:
  You can break `im_str`, if you have a function that takes a `std::string_view` by reference and then assign a different, **unrelated** `std::string_view` into it (NOTE that calling `remove_prefix` or performing a similar operation is fine). Considering that  `std::string_view` is very cheap to copy, it is again hard to imagine a sensible API that would `std::string_view` as an out parameter instead of just returning it by value.
  The danger is a bit bigger for `im_zstr`, as calling `remove_suffix` on a `std::string_view&` that refers to an `im_zstr` would break it's invariant (this is why it is made private in `im_zstr`), as would assigning a subview.

In summary, the advantage (less code to maintain, less inderections to optimize away) seems to outweight the potential dangers at least for now. The design may change in a future version if this should actualy turn out to be a problem in practice, or as a side-effect of implementing SSO.

## No SSO

`im_str` doesn't implement the small string optimization (yet). The main reason is that implementing SSO correctly and efficiently would be a non-trivial effort and at the same time we don't have a good way to measure, how it would impact performance of the code-bases we care about.
Keep in mind, that many of the problems that SSO addresses are not quite as pronounced for `im_str` as for `std::string`:

- The underlying data is never changed, so we don't have to worry about thread safety or implicit unsharing.
- Copying is already very cheap, as it doesn't involve any allocations and in our code base, most small strings are created from string litterals anyway. In those cases, even the initial creation doesn't require an allocation and a copy doesn't trigger a ref-count update (one of the reasons im_str was created in the first place).
- im_str is currently heavily used on memory constraints 32 bit architectures, and it is not clear, what buffer size would provide a good tradeoff between implementation complexity, overall memory footprint (less heap use, but potentially more stack use) and performance (avarage as well as worst-case).

That is not to say that `im_str` wouldn't benefit from SSO, just that the cost/benefit ratio is not clear enought without first having having proper benchmarks in place that would allow a quantification of the benefit.

## No support for standard allocators

Being able to use a custom allocator is actually very important for embedded systems but the standard allocator interface is pretty cumbersome, would require an additional pointer and so far I didn't have the need for different allocators in the same program. If you want to use a different allocation mechanism than new/delete I'd recommend to modify  `detail::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer` and  `detail::atomic_ref_cnt_buffer::atomic_ref_cnt_buffer::_decref` accordingly (which is what we did internally).
In the future, we may add other allocators as an example (maybe behind a typedef).

**It is unlikely that `im_str` will ever receive a allocator template parameter.**


## No support for other char types than `char`

So far, the need to support anything else but plain chars simply hasn't come up yet.

**This may change to `char8_t` in the future, but most likely we will stay in sync with whatever std::string_view does.**


# TODO / Future goals

- Complete docs/readme
- Allow concat to also consume intregral numbers without going through `std::to_string()`

# License

This code is licensed under the MIT licence
