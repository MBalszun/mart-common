#ifndef IM_STR_IM_STR_H
#define IM_STR_IM_STR_H

#include "detail/config.hpp"
#include "detail/ref_cnt_buf.hpp"
#include "detail/string_view_mixin.hpp"

#if IM_STR_USE_CUSTOM_DYN_ARRAY
#include "detail/dynamic_array.hpp"
#else
#include <vector> // used for split_full TODO: move this into separate file
#endif            // IM_STR_USE_CUSTOM_DYN_ARRAY

#include <algorithm>
#include <cassert>
#include <numeric>
#include <string_view>
#include <type_traits>
#include <utility> // tuple/pair

namespace mba {

/**
 * Same as im_str, but guaranteed to be zero terminated
 *
 * Forward declaration, as some im_str member functions return a
 * zero terminated im_str
 */
class im_zstr;

/**
 * im_str is an immutable string class that doesn't allocate
 * when constructed from string litterals
 *
 * Forward declaration, as some im_str member functions return a
 * zero terminated im_str
 */
class im_str : public mba::_detail::str_view_mixin<im_str> {
	using Base_t = mba::_detail::str_view_mixin<im_str>;

protected:
	using Handle_t = _detail_im_str::atomic_ref_cnt_buffer;

public:
#ifdef IM_STR_USE_CUSTOM_DYN_ARRAY
	using DynArray_t = _detail_im_str::dynamic_array<im_str>;
#else
	using DynArray_t = std::vector<im_str>;
#endif
	/* #################### CTORS ################################################################################### */

	// Default ConstString points at empty string
	constexpr im_str() noexcept = default;

	IM_STR_CONSTEXPR_IN_CPP_20 explicit im_str( std::string_view                                   other,
												_detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc = nullptr )
	{
		_copy_from( other, alloc );
	}

	// NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<std::size_t N>
	constexpr im_str( const char ( &other )[N] ) noexcept
		: _view( other )
	// we don't have to copy the data to the freestore as string litterals already have static lifetime
	{
	}

	struct trust_me_this_is_from_a_string_litteral_t {
	};

	static constexpr trust_me_this_is_from_a_string_litteral_t trust_me_this_is_from_a_string_litteral{};

	/**
	 * @brief Create a im_str from a string_view referring to data with static lifetime.
	 *
	 * Usually, when creating im_str from a string_view, it will allocate
	 * the necessary amount of memory on the heap and copy the data over.
	 * data over
	 *
	 *
	 * @param string
	 * @param Tag type
	 * @return
	 */
	constexpr im_str( std::string_view string, trust_me_this_is_from_a_string_litteral_t ) noexcept
		: _view( string )
	{
	}

	// don't accept c-strings in the form of pointer
	// if you need to create a im_str from a c string use the factory function im_str::from_c_str
	template<class T>
	im_str( T const* const& other ) = delete;

	IM_STR_CONSTEXPR_IN_CPP_20 static im_str from_c_str( const char* str )
	{
		return im_str{ std::string_view( str ) };
	};


	/* ############### Special member functions ##################################################################### */
	constexpr im_str( const im_str& other ) noexcept = default;
	constexpr im_str( im_str&& other ) noexcept
		: _view( _detail_im_str::c_expr_exchange( other._as_strview(), std::string_view{} ) )
		, _handle( std::move( other._handle ) )
	{
	}

	// NOTE could be = defaulted in c++20 but needs to be written down explicitly in c++17 in order to be constexpr
	constexpr im_str& operator=( const im_str& other ) noexcept
	{
		this->_view   = other._view;
		this->_handle = other._handle;
		return *this;
	}

	constexpr im_str& operator=( im_str&& other ) noexcept
	{
		this->_view   = _detail_im_str::c_expr_exchange( other._as_strview(), std::string_view{} );
		this->_handle = std::move( other._handle );
		return *this;
	}

	/* ################## String functions  ######################################################################### */
	constexpr operator std::string_view() const { return this->_view; }

	IM_STR_CONSTEXPR_IN_CPP_20 im_str substr( std::size_t offset = 0, std::size_t count = npos ) const& noexcept
	{
		return {
			this->_as_strview().substr( offset, count ), //
			this->_handle                                //
		};
	}

	IM_STR_CONSTEXPR_IN_CPP_20 im_str substr( std::size_t offset = 0, std::size_t count = npos ) && noexcept
	{
		return {
			this->_as_strview().substr( offset, count ), //
			std::move( this->_handle )                   //
		};
	}

	IM_STR_CONSTEXPR_IN_CPP_20 im_str substr( std::string_view range ) const noexcept
	{
		// TODO: strictly speaking those pointer comparisons are UB
		assert( ( data() <= range.data() ) && ( range.data() + range.size() <= data() + size() ) );
		return {
			range,        //
			this->_handle //
		};
	}

	IM_STR_CONSTEXPR_IN_CPP_20 im_str substr( iterator start, iterator end ) const noexcept
	{
		assert( end >= start );
		// UGLY: start-begin()+data() is necessary to convert from an iterator to a pointer
		// if the iterator isn't a pointer
		return substr( std::string_view( start - begin() + data(), static_cast<size_type>( end - start ) ) );
	}

	IM_STR_CONSTEXPR_IN_CPP_20 im_str substr_sentinel( std::size_t offset, char sentinel ) const noexcept
	{
		const auto size = _view.find( sentinel, offset );
		return substr( offset, size - offset );
	}

	/*
	 * Most split functions allow you to specify if the string should be split before or after
	 * the specified characteor or if the char should be dropped completely
	 *
	 * e.g.		auto[f,s] = im_str("Hello World!").split_at(6, im_str:Split::Before); // -> f = "Hello"  s = " World!"
	 *			auto[f,s] = im_str("Hello World!").split_at(6, im_str:Split::After);  // -> f = "Hello " s =  "World!"
	 *			auto[f,s] = im_str("Hello World!").split_at(6, im_str:Split::Drop);   // -> f = "Hello"  s =  "World!"
	 */
	enum class Split { Drop, Before, After };

	// split string into two substrings [0,i) and [i, this->size() )
	[[deprecated( "Use split_at instead" )]] std::pair<im_str, im_str> split( std::size_t i ) const
	{
		return split_at( i );
	}

	// split string into two substrings [0,i) and [i, this->size() )
	IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_at( std::size_t i ) const
	{
		assert( i < size() || i == npos );
		if( i == npos ) { return { *this, {} }; }
		return { substr( 0, i ), substr( i, npos ) };
	}

	// split string into two substrings [0,i) and [i, this->size() )
	IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_at( std::size_t i, Split s ) const
	{
		assert( i < size() || i == npos );
		if( i == npos ) { return { *this, {} }; }
		return { substr( 0, i + ( s == Split::After ) ),
				 substr( i + ( s == Split::After || s == Split::Drop ), npos ) };
	}

	// split string on first occurence of c.
	[[deprecated( "Use split_on_first instead" )]] IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str>
																			  split_first( char c = ' ', Split s = Split::Drop ) const
	{
		return split_on_first( c, s );
	}

	// split string on last occurence of c.
	[[deprecated( "Use split_on_last instead" )]] IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str>
																			 split_last( char c = ' ', Split s = Split::Drop ) const
	{
		return split_on_last( c, s );
	}

	// split string on first occurence of c.
	IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_on_first( char c = ' ', Split s = Split::Drop ) const
	{
		auto pos = _view.find( c );
		return split_at( pos, s );
	}

	// split string on last occurence of c
	IM_STR_CONSTEXPR_IN_CPP_20 std::pair<im_str, im_str> split_on_last( char c = ' ', Split s = Split::Drop ) const
	{
		auto pos = _view.rfind( c );
		return split_at( pos, s );
	}

	/**
	 * @brief  Splits string at each occurence of \p delimiter (dropping the delimiter)
	 *
	 * Example:
	 * auto groups = im_str("123;456;78").split_full(';');
	 * assert( groups[0] == "123" );
	 * assert( groups[1] == "456" );
	 * assert( groups[2] == "58" );
	 *
	 * @param delimiter	 char on which to split
	 *
	 * @return An array containing all substrings. If \p delimiter is not found, the array holds a single entry, which
	 * is a complete copy of this
	 */
	DynArray_t split_full( const char delimiter, const Split s = Split::Drop ) const noexcept
	{
		if( size() == 0 ) { return {}; }

		const auto split_cnt = 1 + std::count( begin(), end(), delimiter );

		DynArray_t ret( split_cnt );
		{
			/* DANGER:
			 * Inside the following loop we create im_str copies of the current im_str, but don't bump the ref count one
			 * by one for efficiency reasons, but only once at the end. In case an exception is thrown midway, we have
			 * to make sure that the already created slices don't decrement the ref-count when they are destructed
			 */

			struct ScopeGuard {
				DynArray_t& slices;
				bool        comitted = false;
				~ScopeGuard()
				{
					if( !comitted ) {
						for( auto& slice : slices ) {
							slice.release();
						}
					}
				}
			} guard{ ret };

			const std::string_view self_view = this->_as_strview();
			std::size_t            start_pos = 0;
			for( auto& slice : ret ) {

				const auto found_pos = _view.find( delimiter, start_pos + (s == Split::Before) );

				slice = im_str(
					// std::string_view::substr(offset,count) allows count to be bigger than size,
					// so we don't have to check for npos here
					self_view.substr( start_pos, found_pos - start_pos + ( s == Split::After ) ),
					_handle,
					_detail_im_str::defer_ref_cnt_tag // ref count will be incremented at the end of the function
				);

				start_pos = found_pos + ( s == Split::Drop || s == Split::After );
			}

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4307 ) // we know that this overflows
#endif
			assert( start_pos == std::string_view::npos + ( std::size_t )( s == Split::Drop || s == Split::After ) );
#ifdef _MSC_VER
#pragma warning( pop )
#endif
			guard.comitted = true;
		}
		_handle.add_ref_cnt( static_cast<int>( split_cnt ) );

		return ret;
	}

	constexpr bool is_zero_terminated() const noexcept { return this->data()[size()] == '\0'; }

	constexpr bool wrapps_a_string_litteral() const noexcept { return _handle == nullptr; }

	/**
	 * This will create a new im_str (actually a im_zstr) whose data resides in a freshly
	 * allocated memory block
	 */
	IM_STR_CONSTEXPR_IN_CPP_20 im_zstr unshare() const;

	/**
	 * Returns a copy if the string is already zero terminated and calls unshare otherwise
	 */
	IM_STR_CONSTEXPR_IN_CPP_20 im_zstr create_zstr() const&;

	/**
	 * Moves "this" into the return value if string is already zero terminated and calls unshare otherwise
	 */
	IM_STR_CONSTEXPR_IN_CPP_20 im_zstr create_zstr() &&;

protected:
	class static_lifetime_tag {
	};

	// used for constructor in im_zstr
	class is_zero_terminated_tag {
	};

	constexpr im_str( std::string_view sv, static_lifetime_tag )  noexcept
		: _view( sv )
	{
	}

	// mostly used in substr
	constexpr im_str( std::string_view sv, const Handle_t& data ) noexcept
		: _view( sv )
		, _handle{ data }
	{
	}

	constexpr im_str( std::string_view sv, Handle_t&& data ) noexcept
		: _view( sv )
		, _handle{ std::move(data) }
	{
	}

	constexpr im_str( std::string_view sv, const Handle_t& data, _detail_im_str::defer_ref_cnt_tag_t ) noexcept
		: _view( sv )
		, _handle{ data, _detail_im_str::defer_ref_cnt_tag_t{} }
	{
	}

	/**
	 * private constructor, that takes ownership of a buffer and a size (used in _copy_from and _concat_impl)
	 */
	constexpr im_str( Handle_t&& handle, const char* data, size_t size )
		: _view( data, size )
		, _handle( std::move( handle ) )
	{
	}

	friend constexpr void swap( im_str& l, im_str& r ) noexcept;

	friend void swap( im_str& l, std::string_view& r ) = delete;
	friend void swap( std::string_view& l, im_str& r ) = delete;

protected:
	std::string_view _view{};
	Handle_t         _handle{};

	friend Base_t;
	constexpr std::size_t _size_for_mixin() const noexcept { return _view.size(); }
	constexpr const char* _data_for_mixin() const noexcept { return _view.data(); }

	constexpr std::string_view&       _as_strview() { return _view; }
	constexpr const std::string_view& _as_strview() const { return _view; }

	constexpr void release() noexcept { _handle.release(); }

	void _copy_from( const std::string_view other, _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc )
	{
		if( other.data() == nullptr ) {
			this->_as_strview() = std::string_view{ "" };
			return;
		}
		// create buffer and copy data over
		auto [data, handle] = Handle_t::allocate_null_terminated_char_buffer( static_cast<int>( other.size() ), alloc );
		std::copy_n( other.data(), other.size(), data );

		// initialize data fields;
		*this = im_str( std::move( handle ), data, other.size() );
	}
};

constexpr void swap( im_str& l, im_str& r ) noexcept
{
	swap( l._handle, r._handle );

	// TODO: in c++20:
	// using std::swap;
	// swap( l._as_strview(), r._as_strview() );  // not yet constexpr
	std::string_view t = l._as_strview();
	l._as_strview()    = r._as_strview();
	r._as_strview()    = t;
}

namespace _detail_im_str_concat {
// ARGS must be std::string_view
template<class... ARGS>
im_zstr variadic_helper( const ARGS... args );

template<class T>
im_zstr range_helper( const T& args );

template<class... ARGS>
im_zstr variadic_helper( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const ARGS... args );

template<class T>
im_zstr range_helper( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args );

} // namespace _detail_im_str_concat

namespace _detail_im_str {
inline constexpr std::string_view getEmptyZeroTerminatedStringView() noexcept
{
	return std::string_view{ "" };
}
} // namespace _detail_im_str

class im_zstr : public im_str {
	using im_str::im_str;

public:
	constexpr im_zstr() noexcept
		: im_str( _detail_im_str::getEmptyZeroTerminatedStringView(), im_str::static_lifetime_tag{} )
	{
	}
	IM_STR_CONSTEXPR_IN_CPP_20 explicit im_zstr( std::string_view other )
		: im_str( other.data() == nullptr ? _detail_im_str::getEmptyZeroTerminatedStringView() : other )
	{
	}

	constexpr im_zstr( const im_str& other, is_zero_terminated_tag ) noexcept
		: im_str( other )
	{
	}

	constexpr im_zstr( im_str&& other, is_zero_terminated_tag ) noexcept
		: im_str( std::move( other ) )
	{
	}

	// NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<std::size_t N>
	constexpr im_zstr( const char ( &other )[N] ) noexcept
		: im_str( other )
	{
	}

	constexpr im_zstr( std::string_view other, trust_me_this_is_from_a_string_litteral_t t ) noexcept
		: im_str( other, t )
	{
	}

	IM_STR_CONSTEXPR_IN_CPP_20 static im_zstr from_c_str( const char* str )
	{
		return im_zstr{ std::string_view( str ) };
	};

	constexpr const char* c_str() const { return this->data(); }

	constexpr bool is_zero_terminated() const noexcept
	{
		assert( im_str::is_zero_terminated() );
		return true;
	}

	friend constexpr void swap( im_zstr& l,
								im_zstr& r ) noexcept; // needs to be defined out of line, otherwise swap(handle,handle)
													   // can't be used in the implemenentation

	friend void swap( im_str& l, im_zstr& r ) = delete;
	friend void swap( im_zstr& l, im_str& r ) = delete;

private:
	/**
	 * private constructor, that takes ownership of a buffer and a size (used in _copy_from and _concat_impl)
	 */
	im_zstr( _detail_im_str::atomic_ref_cnt_buffer&& handle, const char* data, size_t size )
		: im_str( std::move( handle ), data, size )
	{
	}

	template<class... ARGS>
	friend im_zstr _detail_im_str_concat::variadic_helper( const ARGS... args );

	template<class T>
	friend im_zstr _detail_im_str_concat::range_helper( const T& args );

	template<class... ARGS>
	friend im_zstr _detail_im_str_concat::variadic_helper( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc,
														   const ARGS... args );

	template<class T>
	friend im_zstr _detail_im_str_concat::range_helper( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc,
														const T&                                           args );
};

constexpr void swap( im_zstr& l, im_zstr& r ) noexcept
{
	swap( l._handle, r._handle );

	// TODO: in c++20:
	// using std::swap;
	// swap( l._as_strview(), r._as_strview() );  // not yet constexpr
	std::string_view t = l._as_strview();
	l._as_strview()    = r._as_strview();
	r._as_strview()    = t;
}

IM_STR_CONSTEXPR_IN_CPP_20 inline im_zstr im_str::unshare() const
{
	return im_zstr( static_cast<std::string_view>( *this ) );
}

IM_STR_CONSTEXPR_IN_CPP_20 inline im_zstr im_str::create_zstr() const&
{
	if( is_zero_terminated() ) {
		return im_zstr{ { *this }, is_zero_terminated_tag{} }; // just copy
	} else {
		return unshare();
	}
}

IM_STR_CONSTEXPR_IN_CPP_20 inline im_zstr im_str::create_zstr() &&
{
	if( is_zero_terminated() ) {
		return im_zstr{ std::move( *this ), is_zero_terminated_tag{} }; // already zero terminated - just move
	} else {
		return unshare();
	}
}

namespace _detail_im_str_concat {
//######## impl helper for concat ###############
inline void addTo( char*& buffer, const std::string_view str )
{
	buffer = std::copy_n( str.data(), str.size(), buffer );
}

/**
 * Function that can concatenate an arbitrary number of std::string_views
 */
template<class... ARGS>
im_zstr variadic_helper( const ARGS... args )
{
	static_assert( ( std::is_same_v<ARGS, std::string_view> && ... ) );
	const std::size_t newSize = ( 0 + ... + args.size() );

	auto buffer = ::mba::_detail_im_str::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer(
		static_cast<int>( newSize ) );

	auto* tmp_data_ptr = buffer.data;
	( addTo( tmp_data_ptr, args ), ... );

	return im_zstr( std::move( buffer.handle ), buffer.data, newSize );
}

template<class T>
im_zstr range_helper( const T& args )
{
	const std::size_t newSize
		= std::accumulate( args.begin(), args.end(), std::size_t( 0 ), []( std::size_t s, const auto& str ) {
			  return s + std::string_view( str ).size();
		  } );

	auto buffer = ::mba::_detail_im_str::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer(
		static_cast<int>( newSize ) );

	auto* tmp_data_ptr = buffer.data;
	for( auto&& e : args ) {
		addTo( tmp_data_ptr, std::string_view( e ) );
	}

	return im_zstr( std::move( buffer.handle ), buffer.data, newSize );
}

// overloads that use memory resources

template<class... ARGS>
im_zstr variadic_helper( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const ARGS... args )
{
	static_assert( ( std::is_same_v<ARGS, std::string_view> && ... ) );
	const std::size_t newSize = ( 0 + ... + args.size() );

	auto buffer = ::mba::_detail_im_str::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer(
		static_cast<int>( newSize ), alloc );

	auto* tmp_data_ptr = buffer.data;
	( addTo( tmp_data_ptr, args ), ... );

	return im_zstr( std::move( buffer.handle ), buffer.data, newSize );
}

template<class T>
im_zstr range_helper( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args )
{
	const std::size_t newSize
		= std::accumulate( args.begin(), args.end(), std::size_t( 0 ), []( std::size_t s, const auto& str ) {
			  return s + std::string_view( str ).size();
		  } );

	auto buffer = ::mba::_detail_im_str::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer(
		static_cast<int>( newSize ), alloc );

	auto* tmp_data_ptr = buffer.data;
	for( auto&& e : args ) {
		addTo( tmp_data_ptr, std::string_view( e ) );
	}

	return im_zstr( std::move( buffer.handle ), buffer.data, newSize );
}

} // namespace _detail_im_str_concat

template<class ARG1, class... ARGS>
inline auto concat( const ARG1 arg1, const ARGS&... args )
	-> std::enable_if_t<std::is_convertible_v<ARG1, std::string_view>, im_zstr>
{
	static_assert( ( std::is_convertible_v<ARGS, std::string_view> && ... ),
				   "variadic concat can only be used with arguments that can be converted to std::string_view" );
	return _detail_im_str_concat::variadic_helper( std::string_view( arg1 ), std::string_view( args )... );
}
template<class T>
inline auto concat( const T& args ) -> std::enable_if_t<!std::is_convertible_v<T, std::string_view>, im_zstr>
{
	// static_assert( <args_is_a_range> )
	return _detail_im_str_concat::range_helper( args );
}

template<class ARG1, class... ARGS>
inline auto concat( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const ARG1 arg1, const ARGS&... args )
	-> std::enable_if_t<std::is_convertible_v<ARG1, std::string_view>, im_zstr>
{
	static_assert( ( std::is_convertible_v<ARGS, std::string_view> && ... ),
				   "variadic concat can only be used with arguments that can be converted to string_view" );
	return _detail_im_str_concat::variadic_helper( alloc, std::string_view( arg1 ), std::string_view( args )... );
}
template<class T>
inline auto concat( _detail_im_str::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args )
	-> std::enable_if_t<!std::is_convertible_v<T, std::string_view>, im_zstr>
{
	// static_assert( <args_is_a_range> )
	return _detail_im_str_concat::range_helper( alloc, args );
}

static_assert( sizeof( im_str ) <= 3 * sizeof( void* ) );
static_assert( sizeof( im_zstr ) <= 3 * sizeof( void* ) );

} // namespace mba

#endif
