#ifndef IM_STR_IM_STR_H
#define IM_STR_IM_STR_H

#include "detail/config.hpp"
#include "detail/ref_cnt_buf.hpp"

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
class im_str : public std::string_view {
	using Base_t = std::string_view;

protected:
	using Handle_t = detail::atomic_ref_cnt_buffer;

public:
#ifdef IM_STR_USE_CUSTOM_DYN_ARRAY
	using DynArray_t = detail::dynamic_array<im_str>;
#else
	using DynArray_t = std::vector<im_str>;
#endif
	/* #################### CTORS ################################################################################### */

	// Default ConstString points at empty string
	constexpr im_str() noexcept = default;

	IM_STR_CONSTEXPR_DESTRUCTOR explicit im_str( std::string_view                           other,
												 detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc = nullptr )
	{
		_copy_from( other, alloc );
	}

	// NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<std::size_t N>
	constexpr im_str( const char ( &other )[N] ) noexcept
		: std::string_view( other )
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
		: std::string_view( string )
	{
	}

	// don't accept c-strings in the form of pointer
	// if you need to create a im_str from a c string use the explicit conversion to string_view
	template<class T>
	im_str( T const* const& other ) = delete;

	/* ############### Special member functions ##################################################################### */
	constexpr im_str( const im_str& other ) noexcept = default;
	constexpr im_str( im_str&& other ) noexcept
		: std::string_view( detail::c_expr_exchange( other._as_strview(), std::string_view{} ) )
		, _handle( std::move( other._handle ) )
	{
	}

	// NOTE could be = defaulted in c++20 but needs to be written down explicitly in c++17 in order to be constexpr
	constexpr im_str& operator=( const im_str& other ) noexcept
	{
		this->_handle                           = other._handle;
		static_cast<std::string_view&>( *this ) = static_cast<const std::string_view&>( other );
		return *this;
	}

	constexpr im_str& operator=( im_str&& other ) noexcept
	{
		this->_as_strview() = detail::c_expr_exchange( other._as_strview(), std::string_view{} );
		_handle             = std::move( other._handle );
		return *this;
	}

	/* ################## String functions  ######################################################################### */
	IM_STR_CONSTEXPR_DESTRUCTOR im_str substr( std::size_t offset = 0, std::size_t count = npos ) const
	{
		im_str retval;
		retval._as_strview() = this->_as_strview().substr( offset, count );
		retval._handle       = this->_handle;
		return retval;
	}

	IM_STR_CONSTEXPR_DESTRUCTOR im_str substr( std::string_view range ) const
	{
		// TODO: structly speaking those pointer comparisons are UB
		assert( ( data() <= range.data() ) && ( range.data() + range.size() <= data() + size() ) );
		im_str retval;
		retval._as_strview() = range;
		retval._handle       = this->_handle;
		return retval;
	}

	IM_STR_CONSTEXPR_DESTRUCTOR im_str substr( iterator start, iterator end ) const
	{
		// UGLY: start-begin()+data() is necessary to convert from an iterator to a pointer on platforms where they are
		// not the same type
		return substr( std::string_view( start - begin() + data(), static_cast<size_type>( end - start ) ) );
	}

	IM_STR_CONSTEXPR_DESTRUCTOR im_str substr_sentinel( std::size_t offset, char sentinel ) const
	{
		const auto size = this->find( sentinel, offset );
		return substr( offset, size == npos ? this->size() - offset : size - offset );
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
	IM_STR_CONSTEXPR_DESTRUCTOR std::pair<im_str, im_str> split_at( std::size_t i ) const
	{
		assert( i < size() || i == npos );
		if( i == npos ) { return { *this, {} }; }
		return { substr( 0, i ), substr( i, npos ) };
	}

	// split string into two substrings [0,i) and [i, this->size() )
	IM_STR_CONSTEXPR_DESTRUCTOR std::pair<im_str, im_str> split_at( std::size_t i, Split s ) const
	{
		assert( i < size() || i == npos );
		if( i == npos ) { return { *this, {} }; }
		return { substr( 0, i + ( s == Split::After ) ),
				 substr( i + ( s == Split::After || s == Split::Drop ), npos ) };
	}

	// split string on first occurence of c.
	[[deprecated( "Use split_on_first instead" )]] IM_STR_CONSTEXPR_DESTRUCTOR std::pair<im_str, im_str>
																			   split_first( char c = ' ', Split s = Split::Drop ) const
	{
		return split_on_first( c, s );
	}

	// split string on last occurence of c.
	[[deprecated( "Use split_on_last instead" )]] IM_STR_CONSTEXPR_DESTRUCTOR std::pair<im_str, im_str>
																			  split_last( char c = ' ', Split s = Split::Drop ) const
	{
		return split_on_last( c, s );
	}

	// split string on first occurence of c.
	IM_STR_CONSTEXPR_DESTRUCTOR std::pair<im_str, im_str> split_on_first( char c = ' ', Split s = Split::Drop ) const
	{
		auto pos = this->find( c );
		return split_at( pos, s );
	}

	// split string on last occurence of c
	IM_STR_CONSTEXPR_DESTRUCTOR std::pair<im_str, im_str> split_on_last( char c = ' ', Split s = Split::Drop ) const
	{
		auto pos = this->rfind( c );
		return split_at( pos, s );
	}

	DynArray_t split_full( char delimiter ) const noexcept
	{
		if( size() == 0 ) { return {}; }

		const auto split_cnt = 1 + std::count( begin(), end(), delimiter );

		DynArray_t ret( split_cnt );

		const std::string_view self_view = this->_as_strview();

		/* !!! DANGER: THIS LOOP MUST NOT THROW DUE TO THE USE OF detail::defer_ref_cnt_tag_t !!! */
		std::size_t start_pos = 0;
		for( auto& slice : ret ) {

			const auto found_pos = this->find( delimiter, start_pos );
			slice                = im_str(
                // std::string_view::substr(offset,count) allows count to be bigger than size,
                // so we don't have to check for npos here
                self_view.substr( start_pos, found_pos - start_pos ),
                _handle,
                detail::defer_ref_cnt_tag // ref count will be incremented at the end of the function
            );

			start_pos = found_pos + 1u;
		}
		_handle.add_ref_cnt( static_cast<int>( ret.size() ) );

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4307 ) // we know that this overflows

		assert( start_pos == std::string_view::npos + (std::size_t)1 );

#pragma warning( pop )
#else

		assert( start_pos == std::string_view::npos + (std::size_t)1 );

#endif

		return ret;
	}

	constexpr bool is_zero_terminated() const { return this->data()[size()] == '\0'; }

	/**
	 * This will create a new im_str (actuall a im_zstr) whose data resides in a freshly
	 * allocated memory block
	 */
	im_zstr unshare() const;

	/**
	 * Returns a copy if the string is already zero terminated and calls unshare otherwise
	 */
	IM_STR_CONSTEXPR_DESTRUCTOR im_zstr create_zstr() const&;

	/**
	 * Moves "this" into the return value if string is already zero terminated and calls unshare otherwise
	 */
	IM_STR_CONSTEXPR_DESTRUCTOR im_zstr create_zstr() &&;

protected:
	class static_lifetime_tag {
	};

	// used for constructor in im_zstr
	class is_zero_terminated_tag {
	};

	constexpr im_str( std::string_view sv, static_lifetime_tag )
		: std::string_view( sv )
	{
	}

	constexpr im_str( std::string_view                     sv,
					  const detail::atomic_ref_cnt_buffer& data,
					  detail::defer_ref_cnt_tag_t ) noexcept
		: std::string_view( sv )
		, _handle{ data, detail::defer_ref_cnt_tag_t{} }
	{
	}
	/**
	 * private constructor, that takes ownership of a buffer and a size (used in _copy_from and _concat_impl)
	 */
	constexpr im_str( detail::atomic_ref_cnt_buffer&& handle, const char* data, size_t size )
		: std::string_view( data, size )
		, _handle( std::move( handle ) )
	{
	}

protected:
	Handle_t _handle{};

	friend void swap( im_str& l, im_str& r )
	{
		using std::swap;
		swap( l._as_strview(), r._as_strview() );
		swap( l._handle, r._handle );
	}

	constexpr std::string_view& _as_strview() { return static_cast<std::string_view&>( *this ); }

	constexpr const std::string_view& _as_strview() const { return static_cast<const std::string_view&>( *this ); }

	void _copy_from( const std::string_view other, detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc )
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

namespace detail_concat {
// ARGS must be std::string_view
template<class... ARGS>
im_zstr variadic_helper( const ARGS... args );

template<class T>
im_zstr range_helper( const T& args );

template<class... ARGS>
im_zstr variadic_helper( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const ARGS... args );

template<class T>
im_zstr range_helper( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args );

} // namespace detail_concat

namespace detail {
inline constexpr std::string_view getEmptyZeroTerminatedStringView() noexcept
{
	return std::string_view{ "" };
}
} // namespace detail

class im_zstr : public im_str {
	using im_str::im_str;

public:
	constexpr im_zstr() noexcept
		: im_str( detail::getEmptyZeroTerminatedStringView(), im_str::static_lifetime_tag{} )
	{
	}
	IM_STR_CONSTEXPR_DESTRUCTOR explicit im_zstr( std::string_view other )
		: im_str( other.data() == nullptr ? detail::getEmptyZeroTerminatedStringView() : other )
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

	constexpr const char* c_str() const { return this->data(); }

	constexpr bool is_zero_terminated() const noexcept
	{
		assert( im_str::is_zero_terminated() );
		return true;
	}

	constexpr bool wrapps_a_string_litteral() const noexcept { return _handle == nullptr; }

	// Deleted, because this function is inherited from std::string_view and
	// would break im_zstr's invariant of always being zero terminated
	constexpr void remove_suffix( size_type n ) = delete;

private:
	/**
	 * private constructor, that takes ownership of a buffer and a size (used in _copy_from and _concat_impl)
	 */
	im_zstr( detail::atomic_ref_cnt_buffer&& handle, const char* data, size_t size )
		: im_str( std::move( handle ), data, size )
	{
	}

	template<class... ARGS>
	friend im_zstr detail_concat::variadic_helper( const ARGS... args );

	template<class T>
	friend im_zstr detail_concat::range_helper( const T& args );

	template<class... ARGS>
	friend im_zstr detail_concat::variadic_helper( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc,
												   const ARGS... args );

	template<class T>
	friend im_zstr detail_concat::range_helper( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args );
};

inline im_zstr im_str::unshare() const
{
	return im_zstr( static_cast<std::string_view>( *this ) );
}

IM_STR_CONSTEXPR_DESTRUCTOR inline im_zstr im_str::create_zstr() const&
{
	if( is_zero_terminated() ) {
		return im_zstr{ { *this }, is_zero_terminated_tag{} }; // just copy
	} else {
		return unshare();
	}
}

IM_STR_CONSTEXPR_DESTRUCTOR inline im_zstr im_str::create_zstr() &&
{
	if( is_zero_terminated() ) {
		return im_zstr{ std::move( *this ), is_zero_terminated_tag{} }; // already zero terminated - just move
	} else {
		return unshare();
	}
}

namespace detail_concat {
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

	auto buffer
		= ::mba::detail::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( static_cast<int>( newSize ) );

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

	auto buffer
		= ::mba::detail::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( static_cast<int>( newSize ) );

	auto* tmp_data_ptr = buffer.data;
	for( auto&& e : args ) {
		addTo( tmp_data_ptr, std::string_view( e ) );
	}

	return im_zstr( std::move( buffer.handle ), buffer.data, newSize );
}

// overloads that use memory resources

template<class... ARGS>
im_zstr variadic_helper( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const ARGS... args )
{
	static_assert( ( std::is_same_v<ARGS, std::string_view> && ... ) );
	const std::size_t newSize = ( 0 + ... + args.size() );

	auto buffer = ::mba::detail::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer(
		static_cast<int>( newSize ), alloc );

	auto* tmp_data_ptr = buffer.data;
	( addTo( tmp_data_ptr, args ), ... );

	return im_zstr( std::move( buffer.handle ), buffer.data, newSize );
}

template<class T>
im_zstr range_helper( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args )
{
	const std::size_t newSize
		= std::accumulate( args.begin(), args.end(), std::size_t( 0 ), []( std::size_t s, const auto& str ) {
			  return s + std::string_view( str ).size();
		  } );

	auto buffer = ::mba::detail::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer(
		static_cast<int>( newSize ), alloc );

	auto* tmp_data_ptr = buffer.data;
	for( auto&& e : args ) {
		addTo( tmp_data_ptr, std::string_view( e ) );
	}

	return im_zstr( std::move( buffer.handle ), buffer.data, newSize );
}

} // namespace detail_concat

template<class ARG1, class... ARGS>
inline auto concat( const ARG1 arg1, const ARGS&... args )
	-> std::enable_if_t<std::is_convertible_v<ARG1, std::string_view>, im_zstr>
{
	static_assert( ( std::is_convertible_v<ARGS, std::string_view> && ... ),
				   "variadic concat can only be used with arguments that can be converted to std::string_view" );
	return detail_concat::variadic_helper( std::string_view( arg1 ), std::string_view( args )... );
}
template<class T>
inline auto concat( const T& args ) -> std::enable_if_t<!std::is_convertible_v<T, std::string_view>, im_zstr>
{
	// static_assert( <args_is_a_range> )
	return detail_concat::range_helper( args );
}

template<class ARG1, class... ARGS>
inline auto concat( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const ARG1 arg1, const ARGS&... args )
	-> std::enable_if_t<std::is_convertible_v<ARG1, std::string_view>, im_zstr>
{
	static_assert( ( std::is_convertible_v<ARGS, std::string_view> && ... ),
				   "variadic concat can only be used with arguments that can be converted to string_view" );
	return detail_concat::variadic_helper( alloc, std::string_view( arg1 ), std::string_view( args )... );
}
template<class T>
inline auto concat( detail::atomic_ref_cnt_buffer::alloc_ptr_t alloc, const T& args )
	-> std::enable_if_t<!std::is_convertible_v<T, std::string_view>, im_zstr>
{
	// static_assert( <args_is_a_range> )
	return detail_concat::range_helper( alloc, args );
}

static_assert( sizeof( im_str ) <= 3 * sizeof( void* ) );
static_assert( sizeof( im_zstr ) <= 3 * sizeof( void* ) );

} // namespace mba

#endif
