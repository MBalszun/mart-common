#ifndef LIB_MART_COMMON_GUARD_CONST_STRING_H
#define LIB_MART_COMMON_GUARD_CONST_STRING_H
/**
 * ConstString.h (mart-common)
 *
 * Copyright (C) 2016-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: Provides ConstString.h a refcounted implementation of an immutable string
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#ifndef MART_COMMON_USE_IM_STR
#define MART_COMMON_USE_IM_STR 1
#endif

#if MART_COMMON_USE_IM_STR
#include <im_str/im_str.hpp>
#else
#include <algorithm>
#include <atomic>
#include <cassert>
#include <string_view>

/* Proprietary Library Includes */
#include "./cpp_std/memory.h"
#include "./cpp_std/utility.h"

/* Project Includes */
#endif

#include "StringView.h"

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

/**
 * ConstString is a ref-counted String implementation, that doesn't allow the modification of the underlying storage at
 * all.
 *
 * One particular property is that when it is constructed from a "const char [N]" argument it is assumed, that this
 * represents a string litteral, in which case ConstString doesn't perform any copy or dynamic memory allocation and
 * also copying the ConstString will not result in any copies or refcount updates.
 *
 * This header also provides a function that can efficently concatenate multiple string like objects, because it
 * needs only a single dynamic memory allocation
 *
 */
#if MART_COMMON_USE_IM_STR

// NOTE: Deprecated in favor of mba::im_str
class [[deprecated( "Use mba::im_str or mba::im_zstr directly" )]] ConstString : public mba::im_str
{
public:
	using im_str::im_str;
	ConstString( const mba::im_zstr& other )
		: im_str( static_cast<const mba::im_str&>( other ) )
	{
	}

	ConstString( mba::im_zstr && other )
		: im_str( std::move( static_cast<mba::im_str&&>( other ) ) )
	{
	}

	ConstString( const mba::im_str& other )
		: im_str( other )
	{
	}

	ConstString( mba::im_str && other )
		: im_str( std::move( other ) )
	{
	}

	constexpr const char* c_str() const noexcept
	{
		assert( this->is_zero_terminated() );
		return this->data();
	}

	constexpr bool isZeroTerminated() const noexcept { return this->is_zero_terminated(); }

#if !MART_COMMON_STRING_VIEW_USE_STD1
	operator mart::StringView() const noexcept { return mart::StringView( std::string_view( *this ) ); }
#endif

	mba::im_zstr createZStr() const& noexcept { return this->create_zstr(); }

	mba::im_zstr createZStr() && noexcept { return this->create_zstr(); }
};

#else

class ConstString : public StringView {
public:
	/* #################### CTORS ########################## */
	// Default ConstString points at empty string
	ConstString() noexcept
		: StringView( mart::EmptyStringView ) {};

	explicit ConstString( StringView other ) { _copyFrom( other ); }

	explicit ConstString( std::string_view other )
		: ConstString( StringView( other ) )
	{
	}

	struct trust_me_this_is_from_a_string_litteral_t {
	};
	static constexpr trust_me_this_is_from_a_string_litteral_t trust_me_this_is_from_a_string_litteral {};

	constexpr explicit ConstString( std::string_view other, trust_me_this_is_from_a_string_litteral_t ) noexcept
		: StringView( other )
	{
	}

	constexpr operator std::string_view() const noexcept { return this->_as_strview(); }

	// NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<size_t N>
	constexpr ConstString( const char ( &other )[N] ) noexcept
		: StringView( other )
	// we don't have to initialize the shared_ptr to anything as string litterals already have static lifetime
	{
		// if the array is of size 0, it can't be a zero terminated string
		static_assert( N >= 1, "" );
	}

	// don't accept c-strings in the form of pointer
	// if you need to create a const_string from a c string use the <const_string(const char*, size_t)> constructor
	// if you need to create a ConstString form a string literal, use the <ConstString(const char(&other)[N])>
	// constructor
	template<class T>
	ConstString( T const* const& other ) = delete;

	/* ############### Special member functions ######################################## */
	ConstString( const ConstString& other ) noexcept = default;
	ConstString& operator=( const ConstString& other ) noexcept = default;

	ConstString( ConstString&& other ) noexcept
		: StringView( mart::exchange( other._as_strview(), mart::EmptyStringView ) )
		, _data( std::move( other._data ) )
	{
	}
	ConstString& operator=( ConstString&& other ) noexcept
	{
		this->_as_strview() = mart::exchange( other._as_strview(), mart::EmptyStringView );
		_data               = std::move( other._data );
		return *this;
	}

	/* ################## String functions  ################################# */
	// ConstString substr(size_t offset, size_t count) const
	//{
	//	ConstString retval;
	//	//use substr fucntionality from our base class
	//	// and copy pointer underlying storage
	//	retval._as_strview() = this->_as_strview().substr(offset, count);
	//	retval._data = this->_data;
	//	return retval;
	//}

	// ConstString substr(size_t offset) const
	//{
	//	return substr(offset, _size - offset);
	//}

	// NOTE: Currently, const string is always zero terminated
	//	     once substr functionality is added, the actual buffer
	//       will still be zero terminated, but the string, this
	//       instance is nominally referring to might not.
	bool isZeroTerminated() const { return ( *this )[size()] == '\0'; }

	ConstString unshare() const { return ConstString( static_cast<mart::StringView>( *this ) ); }

	ConstString createZStr() const&
	{
		if( isZeroTerminated() ) {
			return *this; // just copy
		} else {
			return unshare();
		}
	}

	ConstString createZStr() &&
	{
		if( isZeroTerminated() ) {
			return std::move( *this ); // already zero terminated - just move
		} else {
			return unshare();
		}
	}

	const char* c_str() const
	{
		assert( isZeroTerminated() );
		return _start;
	}

	template<class... ARGS>
	friend ConstString concat( const ARGS&... args );

	template<class... ARGS>
	friend std::string concat_cpp_str( const ARGS&... args );

private:
	class atomic_ref_cnt {
		using Cnt_t = std::atomic_int;

	public:
		static constexpr size_t required_space = sizeof( Cnt_t );

		atomic_ref_cnt() = default;
		explicit atomic_ref_cnt( std::unique_ptr<char[]>&& location ) noexcept
			: _cnt {new( location.release() ) Cnt_t {1}}
		{
		}
		atomic_ref_cnt( const atomic_ref_cnt& other ) noexcept
			: _cnt {other._cnt}
		{
			_incref();
		}
		atomic_ref_cnt( atomic_ref_cnt&& other ) noexcept
			: _cnt {other._cnt}
		{
			other._cnt = nullptr;
		}
		atomic_ref_cnt& operator=( const atomic_ref_cnt& other ) noexcept
		{
			// inc before dec to protect against self assignment
			other._incref();
			_decref();
			_cnt = other._cnt;

			return *this;
		}
		atomic_ref_cnt& operator=( atomic_ref_cnt&& other ) noexcept
		{
			_decref();
			_cnt       = other._cnt;
			other._cnt = nullptr;
			return *this;
		}
		~atomic_ref_cnt() { _decref(); }

		char*       get() noexcept { return reinterpret_cast<char*>( _cnt ) + sizeof( Cnt_t ); }
		const char* get() const noexcept { return reinterpret_cast<const char*>( _cnt ) + sizeof( Cnt_t ); }

		friend void swap( atomic_ref_cnt& l, atomic_ref_cnt& r ) noexcept { std::swap( l._cnt, r._cnt ); }

	private:
		void _decref() const noexcept
		{
			if( _cnt ) {
				if( _cnt->fetch_sub( 1 ) == 1 ) { delete[]( reinterpret_cast<char*>( _cnt ) ); }
			}
		}
		void _incref() const noexcept
		{
			if( _cnt ) { _cnt->fetch_add( 1, std::memory_order_relaxed ); }
		}
		Cnt_t* _cnt = nullptr;
	};

	atomic_ref_cnt _data;

	// std::shared_ptr<const char> _data = nullptr;

	/** private constructor, that takes ownership of a buffer and a size (used in _copyFrom and _concat_impl)
	 *
	 * Implementation notes:
	 * - creating from shared_ptr doesn't bring any advantage, as you can't use std::make_shared for an array
	 * - This automatically stores the correct deleter in the shared_ptr (by default shared_ptr<const char> would use
	 * delete instead of delete[]
	 */
	ConstString( atomic_ref_cnt&& data, size_t size )
		: StringView( data.get(), size )
		, _data( std::move( data ) )
	{
		assert( _start != nullptr );
	}

	friend void swap( ConstString& l, ConstString& r )
	{
		using std::swap;
		swap( l._as_strview(), r._as_strview() );
		swap( l._data, r._data );
	}

	StringView& _as_strview() { return static_cast<StringView&>( *this ); }

	constexpr const StringView& _as_strview() const { return static_cast<const StringView&>( *this ); }

	static inline atomic_ref_cnt _allocate_null_terminated_char_buffer( size_t size )
	{
		// c++20: std::make_unique_default_init<char[]>( size + 1 + atomic_ref_cnt::required_space )
		atomic_ref_cnt data {std::unique_ptr<char[]>( new char[size + 1 + atomic_ref_cnt::required_space] )};
		data.get()[size] = '\0'; // zero terminate
		return data;
	}

	void _copyFrom( const mart::StringView other )
	{
		if( other.data() == nullptr ) {
			this->_as_strview() = EmptyStringView;
			return;
		}
		// create buffer and copy data over
		auto data = _allocate_null_terminated_char_buffer( other.size() );
		std::copy_n( other.data(), other.size(), data.get() );

		// initialize ConstString data fields;
		*this = ConstString( std::move( data ), other.size() );
	}

	template<class... ARGS>
	static ConstString _concat_impl( ARGS... args );
};
#endif

//######## impl helper for concat ###############
namespace detail {

inline void _addTo( char*& buffer, const std::string_view str )
{
	std::copy_n( str.cbegin(), str.size(), buffer );
	buffer += str.size();
}

template<class... ARGS>
inline void _write_to_buffer( char* buffer, ARGS... args )
{
	( _addTo( buffer, args ), ... );
}

} // namespace detail

#if MART_COMMON_USE_IM_STR

using mba::concat;

#else

/**
 * Function that can concatenate an arbitrary number of objects from which a mart::string_view can be constructed
 * returned constStr will always be zero terminated
 */
template<class... ARGS>
ConstString concat( const ARGS&... args )
{
	return ConstString::_concat_impl( std::string_view( args )... );
}

template<class... ARGS>
inline ConstString ConstString::_concat_impl( ARGS... args )
{
	const size_t newSize = ( 0 + ... + args.size() );

	auto data = _allocate_null_terminated_char_buffer( newSize );

	detail::_write_to_buffer( data.get(), args... );

	return ConstString( std::move( data ), newSize );
}

inline const mart::ConstString& getEmptyConstString()
{
	const static mart::ConstString str( mart::EmptyStringView );
	return str;
}

#endif

namespace detail {
template<class... ARGS>
std::string concat_cpp_str( ARGS... args )
{
	static_assert( ( std::is_same_v<ARGS, std::string_view> && ... ) );
	const size_t newSize = ( 0 + ... + args.size() );

	std::string ret( newSize, ' ' );

	_write_to_buffer( &ret[0], args... );

	return ret;
}

} // namespace detail

template<class... ARGS>
std::string concat_cpp_str( const ARGS&... args )
{
	return detail::concat_cpp_str( std::string_view( args )... );
}

} // namespace mart

#endif