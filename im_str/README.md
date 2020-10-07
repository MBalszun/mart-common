# ImStr [![Build Status](https://travis-ci.org/MBalszun/im_str.svg?branch=master)](https://travis-ci.org/MBalszun/im_str)
### **NOTE: This readme is an early draft. It’s known to be incomplet and incorrekt ;)**

An immutable, ref-counted string class
- Doesn't allocate if constructed from a string litteral.
- Provides convenient split and concatenation functionality.
- Requires c++17
- Optional support for `std::pmr::memory_resource` (but not for `std::pmr::allocator`)
- Can be a constexpr variable in c++20 when constructed from a string litteral.

*) Note "String litteral" means from a `const char c[N]` object, which is blindly assumed to be a string litteral.

# Desciption
The central class of this library is the immutable string type `im_str` and its zero terminated version `im_zstr`. Conceptually it is a `std::string_view` (with some additional member functions like `split`) that either points to a string litteral or owns the data it points to via a shared pointer.

The main motivation behind this type was to have a string type that can be used similar to a std::string, but doesn't require-- on the one hand -- doesn't force allocation and copying if the data it is constructed from has static storage duration anyway (such as string litterals) but that is - on the other hand - transparently takes care of allocation and dealocation when necessary (other than std::string_view).


	#include <im_str/im_str.hpp>
	#include <iostream>

	struct AppConfig {
		mba::im_str log_file_name;
		mba::im_str greeting;
	};

	// in c++20 this can be constexpr of "just" const
	const AppConfig default_config{ "log.txt", "Hello, World! How are you today?" };

	AppConfig make_config( int argc, char** argv )
	{
		if( argc >= 3 ) {
			return AppConfig{
				mba::im_str::from_c_str( argv[1] ),
				mba::im_str::from_c_str( argv[2] )
			};
		} else {
			return default_config;
		}
	}

	void run( const AppConfig& entry )
	{
		std::cout << entry.greeting << std::endl;
	}

	int main( int argc, char** argv )
	{
		auto config = make_config( argc, argv );
		run( config );
	}

# Usage
## Integration

The library is header only so you can just copy the content of the `include` directory to whereever you want it and add the folder to the include path. The other possiblility is to use it as a subdirectory from your cmake file i.e.

    add_subdirectory( <path-to-im_str-root> )
    target_link_libraries( my_app PRIVATE ImStr::im_str )

It currently doesn't support installation and usage via `find_package`


## API overview
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

		im_str cpy = name;
		name       = im_str( "Jane Doe" ); // No allocation here
		assert( cpy == "John" );           // cpy hasn't changed when reassigning the original name variable

		auto [first1, second1] = name.split_on_first( ' ' );
		assert( first1 == "Jane" );
		assert( second1 == "Doe" );

		auto [first2, second2] = im_str( "Hello;World" ).split_on_first( ';', mba::im_str::Split::After );
		assert( first2 == "Hello;" );
		assert( second == "World" );
    }


Even though `im_str` doesn't implement SSO (yet), there isn't a single allocation happening in the above code.
Allocations are only neccesary, when an `im_str` is created from something other than a string litteral or another `im_str`:

    std::string name = "Mike";
    mba::im_str  is  = mba::im_str( name );          // This allocates

    mba::im_str full_greeting = mba::concat( "Hello, ", name, "!\n" ); // This will also allocate (once)

    std::cout << full_greeting; // Prints "Hello, Mike!", followed by a newline

Just like string_view, im_str is not guaranteed to be zero terminated (this can be queried via the member `im_str::is_zero_terminated()` though).
However, the library also provides the type `im_zstr`, which is derived from `im_str` and is guaranteed to be always zero terminated. `concat` actually returns a `im_zstr`

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


## Full API description
### Overview
The library contains the following public types and functions:

- `im_str` : A ref counted immutable string
- `im_zstr` : Derived from `im_str` with the additional guarantee that the string is zero terminated
- `concat` : Free function that creates a `im_zstr` by concatenating an arbitratry number of objects taht can be converted to std::string_view

all of them living in the namespace `mba`.


### `im_str`

#### Construction
-  `IM_STR_CONSTEXPR_IN_CPP_20 explicit im_str( std::string_view                                   other,`
												`_detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc = nullptr )`
	Regular construction from anything that can be converted to a std::string_view. Allocates memory (either via malloc or from the passed std::pmr::memory_resource) and copies the data. Copies are created by bumping a shared reference count.


-  `template<std::size_t N>`
	`constexpr im_str( const char ( &other )[N] ) noexcept`
	Constructor is ONLY meant to be used for construction from a string litteral (i.e. `mba::im_str str("Hello World")`), in which case `im_str` does not allocate memory but just refers to the static string data (as a result there will also be no reference counts during copying).

-	`constexpr im_str( std::string_view view, trust_me_this_is_from_a_string_litteral_t ) noexcept`
	This constructor has the same effect as the second constructor (no allocation, just taking a reference to the existing data).

	DANGER: Be extremely careful when using this constructor. You must ensure that the data `view` refers to outlives this instance of im_str as well as ALL OF ITS COPIES.


#### View interface

- `im_str` provides the usual member type aliases with the following definitions

		using value_type      = const char;
		using pointer         = const char*;
		using const_pointer   = const char*;
		using reference       = const char&;
		using const_reference = const char&;
		using iterator        = const char*;
		using const_iterator  = const char*;

		using reverse_iterator       = std::string_view::reverse_iterator;
		using const_reverse_iterator = std::string_view::const_reverse_iterator;

		using size_type       = std::size_t;
		using difference_type = std::ptrdiff_t;

		static constexpr size_type npos = std::string_view::npos;


- 	The following functions have the same meaning and interface as on `std::string_view`

	- `begin()`, `cbegin()`, `rbegin()`, `crbegin()` and `end()`, `cend()`, `rend()`, `crend()`
	- `size()`, `length()`, `empty()`
	- `data()`, `operator[]()`, `front()`, `back()`

- It can be converted to `std::string_view` either explicitly or via the implicit conversion operator

      constexpr std::string_view to_string_view() const noexcept
	  constexpr operator std::string_view() const noexcept

- It provides the full set of comparison operators (==, !=,  <,>, <=, >=) (implemented as hidden friends) an allows comparison either to itself or a std::string_view object. E.g. the overload set for operator== looks like this:

    constexpr bool operator==( const im_str& l, const im_st& r );
	constexpr bool operator==( std::string_view l, const im_st& r ) ;
	constexpr bool operator==( const im_st& l, std::string_view r );

- It also provides a suitable implementation of `operator<<` in terms of `std::string_view`
- It currently doesn't provide: `at()`

#### Substring

All `substr` overloads create a new `im_str` object that references a subrange of the original string. They only differ in how they specify the subrange.

- `IM_STR_CONSTEXPR_IN_CPP_20 im_str substr( std::size_t offset = 0, std::size_t count = npos ) const noexcept`:

	Substring starts at `offset` and ends at `offset + count`. If `offset + count` is bigger than `size()` or `npos`, the size of the new string is `size()-offset`.

- `IM_STR_CONSTEXPR_IN_CPP_20 im_str substr( std::string_view range ) const noexcept`

	Range must refer to a subrange of the current `im_str` object

- `IM_STR_CONSTEXPR_IN_CPP_20 im_str substr( iterator start, iterator end ) const noexcept`

	`start` and `end`must be iterators into the current `im_str` object

- `IM_STR_CONSTEXPR_IN_CPP_20 im_str substr_sentinel( std::size_t offset, char sentinel ) const noexcept`

	Creates a substring starting at `offset` and ending at the first occurence of `sentinel`

#### Splitting

`im_str` provides multiple splitting functions that produce two substrings. Most of them accept a policy parameter that allows to specify how to handle the char at the split point:

- The available policies are `enum class Split { Drop, Before, After };`:

	-  `im_str::Split::Before`: The character at position i is attached to the second sub-string (`[0,i)` and `[i, size())`)
	-  `im_str::Split::After`: The character at position i is attached to the first sub-string (`[0,i]` and `[i+1, size())`)
	-  `im_str::Split::Drop`: The character at position i is attached to neither substring (the resulting ranges are `[0,i)` and `[i+1, size())`])

-	`IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_at( std::size_t i ) const`:
	Split the string into two substrings `[0, i)` and `[i, size())`

-	`IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_at( std::size_t i, Split s ) const`:
	 Additionally allows to specify the split policy

-	`IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_on_first( char c = ' ', Split s = Split::Drop ) const`:
	Split on first occurence of `c`

-	`IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_on_last( char c = ' ', Split s = Split::Drop ) const`:
	Split on last occurence of `c`

- 	`DynArray_t split_full( const char delimiter, const Split s = Split::Drop ) const noexcept`:
	Splits the string at all occurences of `delimiter` and returns a dynamically allocated collection of substrings

#### Other

- `constexpr bool is_zero_terminated() const noexcept`:
   Returns true if my_im_str[my_im_str.size()] would return '\0'

- `constexpr bool is_string_litteral() const noexcept`:
   Returns true if string was default constructed or is referencing a string litteral

- `im_zstr unshare() const;`
   Creates a truly independent copy in a newly allocated memory region. This can be useful if you want to prevent the reference count to cause synchronization between differnt threads.

- `IM_STR_CONSTEXPR_IN_CPP_20 im_zstr create_zstr() const&`
  `IM_STR_CONSTEXPR_IN_CPP_20 im_zstr create_zstr() &&`:
   Creates a im_zstr object. If is_zero_terminated() was true, this will just cause a ref-bump. If it is false, it will call `unshare()` internally. The r-value overload will be equivalent to a move if is_zero_terminated() is true.


### `im_zstr`

Same as `im_str`, but guaranteed to be zero terminated and hence provides `.c_str()` member. This is e.g. the result from `concat`.


### Concatenation

- `template<class ARG1, class... ARGS>`
  `im_zstr concat( const ARG1 arg1, const ARGS&... args )`:
   Converts each argument into a std::string_view and then creates a string by concatenating the individual arguments. Only takes part in overload resolution, if the first argument is convertible to `std::string_view`

- `template<class T>`
  `im_zstr concat( const T& args )`
   Overload for containers containing a runtime number of string-like objects. Concatenates the elements of `args` to a single string.


- `template<class ARG1, class... ARGS>`
  `im_zstr concat( std::pmr::memory_resource alloc, const ARG1 arg1, const ARGS&... args )`
   Same as first overload, but accepts a custom memory resource that is used to allocate and free memory

- `template<class T>`
  `im_zstr concat( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args )`
   Same as second overload, but accepts a custom memory resource that is used to allocate and free memory

<!--
# Why another string class?

**Disclaimer**: Essentially, this is a simplified, c++17 version of a type that has mainly been used in (semi-) embedded system projects and was developed at a time where the toolchains for those systems either didn't provide a `std::string` at all, or at least not a SSO version. While it can also be usefull in desktop and server applications, the drawback of using a non-standard string type with less micro-optimization compared to modern implementations will likely outweight the benefits.

Two important optimizations compared to `std::string` are that 1) the internal data is ref-counted, so copying is (comparibly) cheap and 2) when constructed from a string litteral (more precisely, when constructed from a const char array) it doesn't allocate at all, but just points to the string litteral. Both of those optimizations are possible because other than `std::string`, `im_str` **doesn't allow mutation of the underlying data**.

In particular the creation from string litterals is a) very efficient and can be performed at compile time and is b) largely transparent to the optimizer. This generally leads to much better/smaller code generation.


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

# (Potential) Controversial Design Decisions
Generally speaking, this library tends to err on the side of simplicity over genericity / flexibility. It is mainly used in a relativley narrow set of applications and I rather optimize for specific usecases than paying the overhead (bet it compile-time, runtime, binary size or code complexity) which often results from overly generic designs whose benefits I'm not (yet) able to reap. Also, the lib is in it's infancy, so I just didn't come around to properly implement some interesting features.

## No SSO

`im_str` doesn't implement the small string optimization (yet). The main reason is that implementing SSO correctly and efficiently would be a non-trivial effort and at the same time we don't have a good way to measure, how it would impact performance of the code-bases we care about.
Keep in mind, that many of the problems that SSO addresses are not quite as pronounced for `im_str` as for `std::string`:

- The underlying data is never changed, so we don't have to worry about thread safety or implicit unsharing.
- Copying is already very cheap, as it doesn't involve any allocations and -- in our code base -- most small strings are created from string litterals anyway. In those cases, even the initial creation doesn't require an allocation and a copy doesn't trigger a ref-count update (one of the reasons im_str was created in the first place).
- im_str is currently heavily used on memory constraints 32 bit architectures, and it is not clear, what buffer size would provide a good tradeoff between implementation complexity, overall memory footprint (less heap use, but potentially more stack use) and performance (avarage as well as worst-case).

That is not to say that `im_str` wouldn't benefit from SSO, just that the cost/benefit ratio is not clear enought without first having having proper benchmarks in place that would allow a quantification of the benefit.

## No support for standard allocators (but for memory_resource)

Being able to use a custom allocator (or to be precise: custom memory management) is actually very important for embedded systems but the standard allocator interface is pretty cumbersome to work with, introduces lots of complexity and so far was not needed in the applications where im_str has been used.
Instead, it was decided to just support the important bit: Custom memory allocation and deallocation via `std::pmr::memory_resource`. Any (potentially) allocating function takes an

## No support for other char types than `char`

So far, the need to support anything else but plain chars simply hasn't come up yet.

**This may change to `char8_t` in the future, but most likely we will stay in sync with whatever std::string_view does.**


# TODO / Future goals

- Complete docs/readme
- Allow concat to also consume intregral numbers without going through `std::to_string()`

# License

This code is licensed under the MIT licence
