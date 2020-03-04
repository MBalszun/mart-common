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
#include <im_str/im_str.hpp>

#include "StringView.h"
#include "./port_layer.h"

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

using mba::concat;

////######## impl helper for concat ###############
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
LIB_MART_COMMON_ALWAYS_INLINE std::string concat_cpp_str( const ARGS&... args )
{
	return detail::concat_cpp_str( std::string_view( args )... );
}

} // namespace mart

#endif