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

using std::common_type_t;
using std::conditional_t;
using std::enable_if_t;
using std::remove_extent_t;
using std::remove_reference_t;
using std::underlying_type_t;

/*####### signed/unsigned ##################*/

using std::make_signed_t;
using std::make_unsigned_t;

/*####### remove cv-qualifiers ##################*/

using std::remove_const_t;
using std::remove_cv_t;
using std::remove_volatile_t;

/*####### add cv-qualifiers ##################*/
using std::add_const_t;
using std::add_cv_t;
using std::add_volatile_t;

using std::decay_t;

using std::conjunction;
using std::conjunction_v;
using std::disjunction;
using std::disjunction_v;

using std::aligned_union;
using std::aligned_union_t;

struct nonesuch {
	nonesuch()                  = delete;
	~nonesuch()                 = delete;
	nonesuch( nonesuch const& ) = delete;
	void operator=( nonesuch const& ) = delete;
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
