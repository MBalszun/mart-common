#ifndef LIB_MART_COMMON_GUARD_CPP_STD_ARRAY_H
#define LIB_MART_COMMON_GUARD_CPP_STD_ARRAY_H
/**
 * array.h (mart-common/cpp_std)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides replacements for standard library symbols in <array>
 *			which are not yet provided by current c++ toolcahin
 *
 */

#include "type_traits.h"

#include <array>
#include <utility>

namespace mart {

// c++14: std::experimental::make_array
// note: not fully compliant, as it doesn't check for std::reference_wrapper
namespace _helper_make_array {
template<class T, class... ARGS>
using return_type = std::array<                      // type is an array of
	std::conditional_t<std::is_same<T, void>::value, // if T is of type void (the default type)
					   std::common_type_t<ARGS...>,  // deduce the element type as common type
					   T                             // otherwise use T as element type
					   >,
	sizeof...( ARGS ) // number of elements is equal to number of parameters
	>;
}

template<class T = void, class... ARGS>
constexpr inline auto make_array( ARGS&&... args ) -> _helper_make_array::return_type<T, ARGS...>
{
	return {std::forward<ARGS>( args )...};
}

} // namespace mart

#endif
