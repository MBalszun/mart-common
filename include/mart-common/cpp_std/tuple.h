#ifndef LIB_MART_COMMON_GUARD_CPP_STD_TUPLE_H
#define LIB_MART_COMMON_GUARD_CPP_STD_TUPLE_H
/**
 * tuple.h (mart-common/cpp_std)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides replacements for standard library symbols in <tuple>
 *			which are not yet provided by current c++ toolcahin
 *
 */

#include <tuple>


namespace mart {

//#### type based get #####
namespace _impl_tbg {
template< class T1, class T2, size_t N, class... Types >
struct TypeGet_t {
	static constexpr size_t getIdx(const std::tuple<Types...>& t) {
		return TypeGet_t<T1, typename std::tuple_element<N + 1, decltype(t)>::type, N + 1, Types... >::getIdx(t);
	}
};

template< class T, size_t N, class... Types >
struct TypeGet_t<T, T, N, Types...> {
	static constexpr size_t getIdx() {
		return N;
	}
};
}

template< class T, class R, class... Types >
constexpr const T& get(const std::tuple<R, Types...>& t) {
	return std::get<_impl_tbg::TypeGet_t <T, R, 0, Types...>::getIdx(t)>(t);
}

}
#endif