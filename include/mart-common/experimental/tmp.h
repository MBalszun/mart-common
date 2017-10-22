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
/* Standard Library Includes */

/* Proprietary Library Includes */
#include "../cpp_std/type_traits.h"
#include "../cpp_std/utility.h"

/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace tmp {


template<class T, std::size_t N>
struct c_array {
	const T t[N];
	constexpr T operator[](size_t i) const {
		return t[i];
	}
};

// get single element from integer_sequence
template <class T, T ... Is>
constexpr T get(std::size_t I,mart::integer_sequence<T,Is...>)
{
	return (c_array<T,sizeof...(Is)>{ { Is... } })[I];
}

namespace detail_cartesian_value_product {
template<
	template<class ... > class Comb,
	class V1, class V2,
	template< V1, V2 > class T,
	class List1,
	class List2,
	std::size_t ... Is
>
auto impl(List1 l1, List2 l2, mart::index_sequence<Is...>)
->Comb < T <
	mart::tmp::get(Is / l2.size(), l1),
	mart::tmp::get(Is % l2.size(), l2)
> ...
>;
}

template<
	template<class ... > class Comb,
	class V1, class V2,
	template< V1, V2 > class T,
	class List1,
	class List2
>
auto cartesian_value_product(List1 l1, List2 l2 )
-> decltype(detail_cartesian_value_product::impl<Comb,V1,V2,T>(l1, l2, mart::make_index_sequence<l1.size()*l2.size()>{}));


}
}

#endif

