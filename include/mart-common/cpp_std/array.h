#ifndef LIB_MART_COMMON_GUARD_CPP_STD_ARRAY_H
#define LIB_MART_COMMON_GUARD_CPP_STD_ARRAY_H

#pragma once

#include <array>
#include <utility>

#include "type_traits.h"

namespace mart {

// c++14: std::experimental::make_array
// c++17: std::make_array
// note: not fully compliant, as it doesn't check for std::reference_wrapper
namespace _helper_make_array {
template <class T, class... ARGS>
using return_type = std::array<						  // type is an array of
	mart::conditional_t<std::is_same<T, void>::value, // if T is of type void (the default type)
						mart::common_type_t<ARGS...>, // deduce the element type as common type
						T							  // otherwise use T as element type
						>,
	sizeof...( ARGS ) // number of elements is equal to number of parameters
	>;
}

template <class T = void, class... ARGS>
constexpr inline auto make_array( ARGS&&... args ) -> _helper_make_array::return_type<T, ARGS...>
{
	return {std::forward<ARGS>( args )...};
}
}

#endif
