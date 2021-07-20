#ifndef LIB_MART_COMMON_GUARD_DYN_ARRAY_H
#define LIB_MART_COMMON_GUARD_DYN_ARRAY_H
/**
 * DynArray.h (mart-common)
 *
 * Copyright (C) 2021: Michael Balszun <michael.balszun@tum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides DynLimArray - a fixed size array with a maximum capacity
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <initializer_list>
#include <memory>

#ifdef __cpp_lib_span
#include <span>
#endif

/* Proprietary Library Includes */
#include "../algorithm.h"

/* Project Includes */
#include "../ArrayViewAdaptor.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

#if __cpp_concepts
namespace detail {

template<class Alloc>
concept AllocWithRealloc = requires( Alloc a )
{
	a.allocate( 5 );
	a.reallocate( (typename Alloc::value_type*)nullptr, 10 );
};

} // namespace detail
#endif

template<class T, class Alloc = std::allocator<T>>
struct DynArrayTriv : mart::ArrayViewAdaptor<T, DynArrayTriv<T, Alloc>> {

	DynArrayTriv() = default;

	DynArrayTriv( std::initializer_list<T> init )
		: _data( Alloc{}.allocate( init.size() ) )
		, _size( init.size() )
	{
		std::copy( init.begin(), init.end(), _data );
	}

	explicit DynArrayTriv( std::size_t size )
		: _data( Alloc{}.allocate( size ) )
		, _size( size )
	{
	}

#ifdef __cpp_lib_span
	explicit DynArrayTriv( std::span<const T> data )
		: DynArrayTriv( data.size() )
	{
		std::copy( data.begin(), data.end(), _data );
	}
#endif

	template<class It1, class It2>
	DynArrayTriv( It1 it_start, It2 it_end )
		: DynArrayTriv( std::distance( it_start, it_end ) )
	{
		std::copy( it_start, it_end, _data );
	}

	DynArrayTriv( const DynArrayTriv& other )
		: DynArrayTriv( other.size() )
	{
		std::copy( other.begin(), other.end(), _data );
	}

	DynArrayTriv( DynArrayTriv&& other )
		: _data( std::exchange( other._data, nullptr ) )
		, _size( std::exchange( other._size, 0 ) )
	{
	}

	DynArrayTriv& operator=( const DynArrayTriv& other )
	{
		if( _size < other._size ) { _data = Alloc::allocate( other._size ); }

		std::copy_n( other._data, other._size, _data );
		_size = other._size;
		return *this;
	}

	DynArrayTriv& operator=( DynArrayTriv&& other )
	{
		_data = std::exchange( other._data, nullptr );
		_size = std::exchange( other._size, 0 );
		return *this;
	}

#if __cpp_concepts
	DynArrayTriv append( const T& t ) && requires detail::AllocWithRealloc<Alloc>
	{
		_data       = Alloc{}.reallocate( _data, _size + 1 );
		_data[_size] = t;
		_size++;
		return std::move( *this );
	}
#endif

	DynArrayTriv append( const T& t ) const
	{
		DynArrayTriv ret( _size + 1 );
		std::copy_n( _data, _size, ret._data );
		ret._data[_size] = t;
		return ret;
	}
	~DynArrayTriv()
	{
		if( _data ) { Alloc{}.deallocate( _data, _size ); };
	}

	constexpr T*          _arrayView_data() noexcept { return _data; }
	constexpr const T*    _arrayView_data() const noexcept { return _data; }
	constexpr std::size_t _arrayView_size() const noexcept { return _size; }

private:
	T*          _data = nullptr;
	std::size_t _size = 0;
};

} // namespace mart

#endif
