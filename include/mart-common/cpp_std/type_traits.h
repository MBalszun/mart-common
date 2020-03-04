#ifndef LIB_MART_COMMON_GUARD_CPP_STD_TYPE_TRAITS_H
#define LIB_MART_COMMON_GUARD_CPP_STD_TYPE_TRAITS_H
/**
 * type_traits.h (mart-common/cpp_std)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides replacements for standard library symbols in <type_traits>
 *			which are not yet provided by current c++ toolcahin
 *
 */

#include <type_traits>

namespace mart {

struct nonesuch {
	nonesuch()                  = delete;
	~nonesuch()                 = delete;
	nonesuch( nonesuch const& ) = delete;
	nonesuch( nonesuch&& )      = delete;
	nonesuch& operator=( nonesuch const& ) = delete;
	nonesuch& operator=( nonesuch&& ) = delete;
};

/*######### Detection Idiom #########*/

// http://en.cppreference.com/w/cpp/experimental/is_detected

namespace detail_detection {
template<class Default, class AlwaysVoid, template<class...> class Op, class... Args>
struct detector {
	using value_t = std::false_type;
	using type    = Default;
};

template<class Default, template<class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
	// Note that std::void_t is a C++17 feature
	using value_t = std::true_type;
	using type    = Op<Args...>;
};

} // namespace detail_detection

template<template<class...> class Op, class... Args>
using is_detected = typename detail_detection::detector<nonesuch, void, Op, Args...>::value_t;

template<template<class...> class Op, class... Args>
using detected_t = typename detail_detection::detector<nonesuch, void, Op, Args...>::type;

template<class Default, template<class...> class Op, class... Args>
using detected_or = detail_detection::detector<Default, void, Op, Args...>;

} // namespace mart
#endif
