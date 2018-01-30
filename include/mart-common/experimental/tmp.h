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
	static constexpr size_t size() { return N; }
};

template<class T, T ... Is, template<class, T...> class sequence>
constexpr c_array<T, sizeof...(Is)> to_carray(sequence<T, Is...>) {
	return { {Is...}};
}

template<class T, T ...Is >
constexpr T get_Nth_element(size_t Idx, mart::integer_sequence<T, Is ...> sequ) {
	return to_carray(sequ)[Idx];
}

template<class T, class ... ARGS >
constexpr decltype(auto) first(T&& t, ARGS ...) {
	return std::forward<T>(t);
}

#ifndef _MSC_VER
// watch out: on gcc, you can't use parameters itself to generate the returntype, but have to create a new value of the type
namespace detail_cartesian_value_product {
template<
	template<class ... > class Comb,
	class V1, class V2,
	template< V1, V2 > class T,
	class List1,
	class List2,
	std::size_t ... Is
>
auto impl(List1, List2, mart::index_sequence<Is...>)
-> Comb < T <
	to_carray(List1{})[Is / List2::size()],
	to_carray(List2{})[Is % List2::size()]
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
-> decltype(detail_cartesian_value_product::impl<Comb,V1,V2,T>(l1, l2, mart::make_index_sequence<List1::size()*List2::size()>{}));
#endif // !_MSVC


}
}

#endif

