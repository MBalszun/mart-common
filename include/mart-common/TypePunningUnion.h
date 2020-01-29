#ifndef LIB_MART_COMMON_GUARD_TYPE_PUNNING_UNION_H
#define LIB_MART_COMMON_GUARD_TYPE_PUNNING_UNION_H
/**
 * TypePunningUnion.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: A type similar to union that can be used as a rx buffer in networking and safely(standard compliant) cast the
 * bytes to a pod
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <cstdint>
#include <type_traits>

/* Proprietary Library Includes */
#include "ArrayView.h"

/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

template<class... Types>
class TypePunningUnion {
public:
	// ###### Helper ######
	template<class T>
	struct is_compatible {
		static constexpr bool value = std::is_standard_layout<T>::value && std::is_trivially_destructible<T>::value
									  && std::is_trivially_copyable<T>::value;
	};

	template<class T>
	static constexpr bool is_member_type()
	{
		return ( std::is_same_v<T, Types> || ... || false );
	}

	static_assert( std::conjunction<is_compatible<Types>...>::value, "" );

	// ###### Special member functions #####
	TypePunningUnion()                                = default;
	TypePunningUnion( const TypePunningUnion& other ) = default;
	TypePunningUnion& operator=( const TypePunningUnion& other ) = default;

	// ###### construction from POD #####
	template<class T>
	TypePunningUnion( const T& msg )
	{
		static_assert( is_member_type<T>(), "Can't assign type that is not member of the union" );
		new( &data ) T( msg );
		_size = sizeof( msg );
	}

	template<class T>
	TypePunningUnion& operator=( const T& msg )
	{
		static_assert( is_member_type<T>(), "Can't assign type that is not member of the union" );
		new( &data ) T( msg );
		_size = sizeof( msg );
		return *this;
	}

	TypePunningUnion& operator=( mart::ConstMemoryView new_data )
	{
		assert( new_data.size() <= sizeof( data ) && "Buffer not big enough" );
		mart::copy( new_data, all_bytes() );
		_size = new_data.size();
		return *this;
	}

	// ###### Rest #####
	// we can't be const correct here, as casting implies (conceptually) copying data forward and backwward in the
	// buffer
	template<class T>
	T& as()
	{
		static_assert( is_member_type<T>(), "" );
		assert( _size >= sizeof( T ) && "Stored object is smaller than the type you want to cast it to" );
		return inplace_cast_to<T>( data );
	}

	std::size_t size() const { return _size; }
	bool        empty() const { return size() == 0; }

	// used, to tell the buffer which bytes have been filled with active data, after the
	// result of all_bytes() has been filled e.g. from the network.
	void set_active_range( mart::MemoryView range )
	{
		if( range.isValid() ) { assert( range.begin() == all_bytes().begin() ); }
		_size = range.size();
	}

	mart::MemoryView      all_bytes() { return mart::view_bytes_mutable( data ); }
	mart::ConstMemoryView all_bytes() const { return mart::view_bytes( data ); }

	mart::MemoryView      valid_bytes() { return all_bytes().subview( 0, _size ); }
	mart::ConstMemoryView valid_bytes() const { return all_bytes().subview( 0, _size ); }

protected:
	typename std::aligned_union<0, Types...>::type data;
	std::size_t                                    _size = 0;

	template<class T>
	static void copy_bytes( const void* from, void* to )
	{
		std::copy_n(
			reinterpret_cast<const unsigned char*>( from ), sizeof( T ), reinterpret_cast<unsigned char*>( to ) );
	}

	// the actual magic
	template<class T, class B>
	static T& inplace_cast_to( B& data )
	{
		// c++11:
		static_assert( is_compatible<T>::value, "" );

		// 1) create uninitialized local temporary of correct type
		// 2) copy bytes from buffer to temporary
		// 3) use placement new to create object in buffer
		// 4) copy bytes back from temporary into buffer
		//
		// Result:
		//	- We don't violate aliasing rules as we create an actual object of type T in the storage
		//  - Compiler will see that we don't actually change the bytes stored at <data> and
		//		optimize away the whole function

		T tbuf;
		copy_bytes<T>( &data, &tbuf );
		auto* const ptr = new( &data ) T;
		copy_bytes<T>( &tbuf, &data );

		// NOTE: We MUST use the pointer returned by new.
		// A reinterpret cast of &data is not ok (see explanation about std::launder)
		return *ptr;
	}
};

} // namespace mart

#endif