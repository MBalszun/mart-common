#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_BUFFER_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_BUFFER_H
/**
 * MultitypeBuffer.h (mart-common/experimental/MultiTypeBuffer.h)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  This header provides a byte buffer which allows casting of raw bytes to different objects
 *
 */

#include "../../ArrayView.h"
#include "../../cpp_std/type_traits.h"

#include <cstdint>
#include <type_traits>

namespace mart {
namespace experimental {

namespace detail {

static constexpr size_t cmax( size_t l, size_t r )
{
	return l < r ? r : l;
}

template <class ARG1, class... ARGS>
static constexpr size_t cmax( ARG1 arg1, ARGS... args )
{
	return cmax(arg1, cmax( args... ));
}

template <class Type1, class Type2>
constexpr bool cisOneOf(Type1, Type2)
{
	return std::is_same<Type1, Type2>::value;
}

template <class Type1, class Type2, class Type3, class... Types>
constexpr bool cisOneOf(Type1 t1, Type2, Type3 t3, Types... t)
{
	return std::is_same<Type1, Type2>::value || cisOneOf<Type1, Type3, Types...>(t1,t3,t...);
}

} //detail

template <class... Types>
class MultitypeBuffer {
	using Storage_t =
		typename std::aligned_storage<detail::cmax( sizeof( Types )... ), detail::cmax( alignof( Types )... )>::type;

	template<class T>
	struct is_compatible {
		static constexpr bool value = std::is_standard_layout<T>::value;
			// c++11: || std::is_trivially_destructible<T>::value
			// c++11: || std::is_trivially_copyable<T>::value
	};
	static_assert(mart::conjunction<is_compatible<Types> ...>::value, "");
public:
	mart::MemoryView	  allBytes() { return mart::viewMemory( _data ); }
	mart::ConstMemoryView allBytes() const { return mart::viewMemoryConst( _data ); }

	mart::MemoryView	  validBytes() { return _valid_bytes; }
	mart::ConstMemoryView validBytes() const { return _valid_bytes; }

	void operator=( mart::ConstMemoryView mem )
	{
		if( mem.size() > sizeof( _data ) ) {
			_valid_bytes = mart::MemoryView{};
			return;
		}
		if( mem.data() == (mart::ByteType*)&_data ) {
			_valid_bytes = mart::MemoryView((mart::ByteType*)&_data, mem.size());
		} else {
			_valid_bytes = {(mart::ByteType*)&_data, std::copy( mem.begin(), mem.end(), (mart::ByteType*)&_data )};
		}
	}

	template <class T>
	T& as()
	{
		return inplace_cast<T>( _data );
	}

private:
	Storage_t		 _data;
	mart::MemoryView _valid_bytes;
	int				 idx = -1;

	/**
	 * Casting data in storage into object of type T, by first copying the bytes
	 * into an appropriate temporary variable and then using placement new to create
	 * an uninitialized object in the original place
	 */
	template <class T>
	static T& inplace_cast( Storage_t& data )
	{
		// c++11: static_assert(std::is_trivially_copyable<T>::value, "");
		static_assert( std::is_standard_layout<T>::value, "" );
		static_assert( sizeof( T ) <= sizeof( data ), "" );
		//static_assert( detail::cisOneOf(std::declval<T>(), std::declval<Types>()...), "" );
		static_assert(mart::disjunction<std::is_same<T,Types> ...>::value, "Type is not a member of the buffer");

		//T tbuf;
		std::array<mart::ByteType, sizeof(sizeof(T))> tbuf;
		// copy data to temporary buffer of correct type
		std::copy_n( reinterpret_cast<const unsigned char*>( &data ),
					 sizeof( tbuf ),
					 reinterpret_cast<unsigned char*>( &tbuf ) );

		// use placement new to create new (but unninitialized) object in data array
		auto* ptr = new( reinterpret_cast<void*>( &data ) ) T;

		// copy data back to the original location
		std::copy_n( reinterpret_cast<const unsigned char*>( &tbuf ),
					 sizeof( tbuf ),
					 reinterpret_cast<unsigned char*>( &data ) );

		return *ptr;
	}
};
}
}

#endif
