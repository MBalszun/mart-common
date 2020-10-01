#ifndef LIB_MART_COMMON_GUARD_DYN_LIM_ARRAY_H
#define LIB_MART_COMMON_GUARD_DYN_LIM_ARRAY_H
/**
 * DynLimArray.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
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

#include <array>
#include <utility>
#include <stdexcept>

/* Proprietary Library Includes */
#include "../algorithm.h"

/* Project Includes */
#include "../ArrayViewAdaptor.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

template<class T, int Capacity>
class DynLimArray : public ArrayViewAdaptor<T, DynLimArray<T, Capacity>> {
public:
	DynLimArray()                           = default;
	DynLimArray( const DynLimArray& other ) = default;
	DynLimArray( DynLimArray&& other )      = default;
	DynLimArray& operator=( const DynLimArray& other ) = default;
	DynLimArray& operator=( DynLimArray&& other ) = default;

	template<class InputIt, class = decltype( *std::declval<InputIt>() )>
	DynLimArray( InputIt begin, InputIt end )
		: _size( static_cast<int>( end - begin ) )
	{
		throwIfTooBig( _size );
		std::copy( begin, end, _data.begin() );
	}

	template<class C, class = decltype( std::declval<C>().end() - std::declval<C>().begin() )>
	explicit DynLimArray( const C& container )
		: _size( container.size() )
	{
		throwIfTooBig( _size );
		std::copy( container.begin(), container.end(), _data.begin() );
	}

	explicit DynLimArray( int size )
		: _size( size )
	{
		throwIfTooBig( _size );
	}

	DynLimArray( std::initializer_list<T> init )
		: _size( init.size() )
	{
		throwIfTooBig( _size );
		mart::copy( init, _data.begin() );
	}

	template<int OtherCapacity>
	DynLimArray( const DynLimArray<T, OtherCapacity>& other )
		: _size( other._size )
	{
		throwIfTooBig( _size );
		mart::copy( other._data, _data.begin() );
	}

	template<int OtherCapacity>
	DynLimArray( DynLimArray<T, OtherCapacity>&& other )
		: _size( other._size )
	{
		throwIfTooBig( _size );
		mart::move( std::move( other._data ), _data.begin() );
	}

	template<int OtherCapacity>
	DynLimArray& operator=( const DynLimArray<T, OtherCapacity>& other )
	{
		throwIfTooBig( other._size );
		_size = other._size;
		mart::copy( other._data, _data.begin() );
	}

	template<int OtherCapacity>
	DynLimArray& operator=( DynLimArray<T, OtherCapacity>&& other )
	{
		throwIfTooBig( other._size );
		_size = other._size;
		mart::move( std::move( other._data ), _data.begin() );
	}

protected:
	void throwIfTooBig( int size ) const
	{
		if( size > Capacity ) { throw std::out_of_range( "Data size exceeds capacity of DynLimArray" ); }
	}

	int                     _size;
	std::array<T, Capacity> _data{};

	friend class ArrayViewAdaptor<T, DynLimArray<T, Capacity>>;
	// interface for ArrayViewAdaptor
	T*                 _arrayView_data() noexcept { return _data.data(); }
	constexpr const T* _arrayView_data() const noexcept { return _data.data(); }
	constexpr int      _arrayView_size() const noexcept { return _size; }
};

} // namespace mart

#endif
