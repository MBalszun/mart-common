#ifndef LIB_MART_COMMON_GUARD_TMP_H
#define LIB_MART_COMMON_GUARD_TMP_H
/**
 * tmp.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: template meta programming helpers
 *
 */

/* ######## INCLUDES ######### */
/* Proprietary Library Includes */
#include "../cpp_std/type_traits.h"

/* Standard Library Includes */
#include <cstdint>

/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace tmp {

template<class T, std::size_t N>
struct c_array {
	const T t[N];

	constexpr T operator[]( std::size_t i ) const { return t[i]; }

	static constexpr auto size() -> std::size_t { return N; }
};

template<class T, T... Is, template<class, T...> class sequence>
constexpr c_array<T, sizeof...( Is )> to_carray( sequence<T, Is...> )
{
	return {{Is...}};
}

template<class T, T... Is>
constexpr T get_Nth_element( std::size_t Idx, std::integer_sequence<T, Is...> sequ )
{
	return to_carray( sequ )[Idx];
}

template<class T, class... ARGS>
constexpr decltype( auto ) first( T&& t, ARGS... )
{
	return std::forward<T>( t );
}

// watch out: on gcc, you can't use parameters itself to generate the returntype, but have to create a new value of the
// type
namespace detail {
template<template<class...> class Comb, // new list type template
		 class V1,                      // types of elements in list1
		 class V2,                      // types of elements in list2
		 template<V1, V2>
		 class T, // template for elements in combined list
		 class List1,
		 class List2,
		 std::size_t... Is>
auto cartesian_value_product( List1, List2, std::index_sequence<Is...> )
{
	// clang-format off
	return Comb<
				T<
					to_carray( List1{} )[Is / List2::size()],
					to_carray( List2{} )[Is % List2::size()]
				>
				...
			>{};
	// clang-format on
}
} // namespace detail

template<template<class...> class Comb, // new list type template
		 class V1,                      // types of elements in list1
		 class V2,                      // types of elements in list2
		 template<V1, V2>
		 class T, // template for of elements in combined list
		 class List1,
		 class List2>
auto cartesian_value_product( List1 l1, List2 l2 )
{
	return detail::cartesian_value_product<Comb, V1, V2, T>(
		l1, l2, std::make_index_sequence<List1::size() * List2::size()>{} );
};

template<class T, T... VALS>
struct value_list {
	static constexpr std::size_t size() { return sizeof...( VALS ); };
};

template<class T, T... Is>
constexpr T get_Nth_element( std::size_t Idx, value_list<T, Is...> sequ )
{
	return to_carray( sequ )[Idx];
}

} // namespace tmp
} // namespace mart

#endif
