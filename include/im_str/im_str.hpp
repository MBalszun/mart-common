#ifndef IM_STR_IM_STR_H
#define IM_STR_IM_STR_H

#include "detail/ref_cnt_buf.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <string_view>
#include <utility> // tuple/pair

#define IM_STR_USE_CUSTOM_DYN_ARRAY
#ifdef IM_STR_USE_CUSTOM_DYN_ARRAY
#include "detail/dynamic_array.hpp"
#else
#include <vector> // used for split_full TODO: move this into separate file
#endif            // IM_STR_USE_CUSTOM_DYN_ARRAY

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
	/* #################### CTORS ########################## */
	// Default ConstString points at empty string
	constexpr im_str() noexcept = default;

	im_str( std::string_view other ) { _copy_from( other ); }

	// NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<std::size_t N>
	constexpr im_str( const char ( &other )[N] ) noexcept
		: std::string_view( other )
	// we don't have to copy the data to the freestore as string litterals already have static lifetime
	{
	}

	// don't accept c-strings in the form of pointer
	// if you need to create a im_str from a c string use the explicit conversion to string_view
	template<class T>
	im_str( T const* const& other ) = delete;

	/* ############### Special member functions ######################################## */
	im_str( const im_str& other ) noexcept = default;
	im_str& operator=( const im_str& other ) noexcept = default;

	im_str( im_str&& other ) noexcept
		: std::string_view( std::exchange( other._as_strview(), std::string_view {} ) )
		, _handle( std::move( other._handle ) )
	{
	}

	im_str& operator=( im_str&& other ) noexcept
	{
		this->_as_strview() = std::exchange( other._as_strview(), std::string_view {} );
		_handle             = std::move( other._handle );
		return *this;
	}

	/* ################## String functions  ################################# */
	im_str substr( std::size_t offset = 0, std::size_t count = npos ) const
	{
		im_str retval;
		retval._as_strview() = this->_as_strview().substr( offset, count );
		retval._handle       = this->_handle;
		return retval;
	}

	im_str substr( std::string_view range ) const
	{
		assert( data() <= range.data() && range.data() + range.size() <= data() + size() );
		im_str retval;
		retval._as_strview() = range;
		retval._handle       = this->_handle;
		return retval;
	}

	im_str substr( iterator start, iterator end ) const
	{
		// UGLY: start-begin()+data() is necessary to convert from an iterator to a pointer on platforms where they are
		// not the same type
		return substr( std::string_view( start - begin() + data(), static_cast<size_type>( end - start ) ) );
	}

	im_str substr_sentinel( std::size_t offset, char sentinel ) const
	{
		const auto size = this->find( sentinel, offset );
		return substr( offset, size == npos ? this->size() - offset : size - offset );
	}

	enum class Split { Drop, Before, After };

	std::pair<im_str, im_str> split( std::size_t i ) const
	{
		assert( i < size() || i == npos );
		if( i == npos ) { return {*this, {}}; }
		return {substr( 0, i ), substr( i, npos )};
	}

	std::pair<im_str, im_str> split( std::size_t i, Split s ) const
	{
		assert( i < size() || i == npos );
		if( i == npos ) { return {*this, {}}; }
		return {substr( 0, i + ( s == Split::After ) ), substr( i + ( s == Split::After || s == Split::Drop ), npos )};
	}

	std::pair<im_str, im_str> split_first( char c = ' ', Split s = Split::Drop ) const
	{
		auto pos = this->find( c );
		return split( pos, s );
	}

	std::pair<im_str, im_str> split_last( char c = ' ', Split s = Split::Drop ) const
	{
		auto pos = this->rfind( c );
		return split( pos, s );
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

			start_pos = found_pos + 1;
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
	im_zstr create_zstr() const&;

	/**
	 * Moves "this" into the return value if string is already zero terminated and calls unshare otherwise
	 */
	im_zstr create_zstr() &&;

protected:
	class static_lifetime_tag {
	};
	constexpr im_str( std::string_view sv, static_lifetime_tag )
		: std::string_view( sv )
	{
	}

	constexpr im_str( std::string_view                     sv,
					  const detail::atomic_ref_cnt_buffer& data,
					  detail::defer_ref_cnt_tag_t ) noexcept
		: std::string_view( sv )
		, _handle {data, detail::defer_ref_cnt_tag_t {}}
	{
	}
	/**
	 * private constructor, that takes ownership of a buffer and a size (used in _copy_from and _concat_impl)
	 */
	im_str( detail::atomic_ref_cnt_buffer&& handle, const char* data, size_t size )
		: std::string_view( data, size )
		, _handle( std::move( handle ) )
	{
	}

private:
	Handle_t _handle {};

	friend void swap( im_str& l, im_str& r )
	{
		using std::swap;
		swap( l._as_strview(), r._as_strview() );
		swap( l._handle, r._handle );
	}

	std::string_view& _as_strview() { return static_cast<std::string_view&>( *this ); }

	const std::string_view& _as_strview() const { return static_cast<const std::string_view&>( *this ); }

	void _copy_from( const std::string_view other )
	{
		if( other.data() == nullptr ) {
			this->_as_strview() = std::string_view {""};
			return;
		}
		// create buffer and copy data over
		auto [data, handle] = Handle_t::allocate_null_terminated_char_buffer( static_cast<int>( other.size() ) );
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
} // namespace detail_concat

namespace detail {
inline constexpr std::string_view getEmptyZeroTerminatedStringView() noexcept
{
	return std::string_view {""};
}
} // namespace detail

class im_zstr : public im_str {
	using im_str::im_str;

public:
	constexpr im_zstr()
		: im_str( detail::getEmptyZeroTerminatedStringView(), im_str::static_lifetime_tag {} )
	{
	}
	im_zstr( std::string_view other )
		: im_str( other.data() == nullptr ? detail::getEmptyZeroTerminatedStringView() : other )
	{
	}

	explicit im_zstr( const im_str& other )
		: im_str( other.create_zstr() )
	{
	}

	im_zstr( im_str&& other )
		: im_str( std::move( other ).create_zstr() )
	{
	}

	// NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<std::size_t N>
	constexpr im_zstr( const char ( &other )[N] ) noexcept
		: im_str( other )
	{
	}
	constexpr const char* c_str() const { return this->data(); }

	constexpr bool is_zero_terminated() const noexcept
	{
		assert( im_str::is_zero_terminated() );
		return true;
	}

private:
	// This function is inherited from std::string_view and would break im_zstr's invariant of always being
	// zero terminated
	constexpr void remove_suffix( size_type n );
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
};

inline im_zstr im_str::unshare() const
{
	return im_zstr( static_cast<std::string_view>( *this ) );
}

inline im_zstr im_str::create_zstr() const&
{
	if( is_zero_terminated() ) {
		return *this; // just copy
	} else {
		return unshare();
	}
}

inline im_zstr im_str::create_zstr() &&
{
	if( is_zero_terminated() ) {
		return std::move( *this ); // already zero terminated - just move
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

} // namespace detail_concat

template<class ARG1, class... ARGS>
inline auto concat( const ARG1 arg1, const ARGS&... args )
	-> std::enable_if_t<std::is_convertible_v<ARG1, std::string_view>, im_zstr>
{
	static_assert( ( std::is_convertible_v<ARGS, std::string_view> && ... ),
				   "variadic concat can only be used with arguments that can be converted to string_view" );
	return detail_concat::variadic_helper( std::string_view( arg1 ), std::string_view( args )... );
}
template<class T>
inline auto concat( const T& args ) -> std::enable_if_t<!std::is_convertible_v<T, std::string_view>, im_zstr>
{
	// static_assert( <args_is_a_range> )
	return detail_concat::range_helper( args );
}

inline const im_str& getEmptyConstString()
{
	const static im_str str {};
	return str;
}

static_assert( sizeof( im_str ) <= 3 * sizeof( void* ) );
static_assert( sizeof( im_zstr ) <= 3 * sizeof( void* ) );

} // namespace mba

#endif
