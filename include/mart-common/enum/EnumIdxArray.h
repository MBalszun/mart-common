#ifndef LIB_MART_COMMON_GUARD_ENUM_ENUM_IDX_ARRAY_H
#define LIB_MART_COMMON_GUARD_ENUM_ENUM_IDX_ARRAY_H
/**
 * EnumIdxArray.h (mart-common/enum)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: array that can directly be indexed by an enum
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
#include "EnumHelpers.h"
#include "../ArrayViewAdaptor.h"

/* Proprietary Library Includes */
/* Standard Library Includes */
#include <type_traits>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
/**
 * encapsulates an array, such that it can be indexed by enum values.
 * Most member functions are directly mapped to their std::array counterparts
 *
 * NOTE: Currently the enums have to be values that directly translate to 0-based indexes
 */
template<class T, class EnumT, std::size_t N = mart::enumCnt<EnumT>()>
struct EnumIdxArray : ArrayViewAdaptor<T, EnumIdxArray<T, EnumT, N>> {
	using Base_t    = ArrayViewAdaptor<T, EnumIdxArray<T, EnumT, N>>;
	using Storage_t = T[N];

	// only explicitly importing types used in the new memberfunctions to reduce typing
	//(user will have automatically access to all member types of the base
	using typename Base_t::const_reference;
	using typename Base_t::reference;
	using typename Base_t::size_type;

	/* ###### CTORS ###### */
	constexpr EnumIdxArray() = default;
	constexpr EnumIdxArray( const EnumIdxArray& other )  = default;
	constexpr EnumIdxArray( EnumIdxArray&& )             = default;
	constexpr EnumIdxArray& operator=( const EnumIdxArray& ) = default;
	constexpr EnumIdxArray& operator=( EnumIdxArray&& ) = default;

	// This is a constructor forwarding the arguments to the internal array,
	// but we have to make sure it doesn't superseed the copy constructor -> hence the enable_if
	template<class A1,
			 class... ARGS,
			 class = std::enable_if_t<!std::is_same<EnumIdxArray<T, EnumT, N>, std::decay_t<A1>>::value>>
	constexpr EnumIdxArray( A1&& arg, ARGS&&... args )
		: _storage{ std::forward<A1>( arg ), std::forward<ARGS>( args )... }
	{
	}

private:
	/*#### data ####*/
	static constexpr auto toIdx( EnumT e ) -> std::size_t
	{
		return static_cast<size_type>( static_cast<std::underlying_type_t<EnumT>>( e ) );
	};

	Storage_t _storage{};

public:
	constexpr T*          _arrayView_data() noexcept { return _storage; }
	constexpr const T*    _arrayView_data() const noexcept { return _storage; }
	constexpr std::size_t _arrayView_size() const noexcept { return N; }

	constexpr reference       operator[]( EnumT pos ) { return _storage[toIdx( pos )]; }
	constexpr const_reference operator[]( EnumT pos ) const { return _storage[toIdx( pos )]; }

	constexpr void fill( const T& value )
	{
		for( auto& e : _storage ) {
			e = value;
		};
	}
};

} // namespace mart

#endif // LIB_MART_COMMON_GUARD_UTILS_H