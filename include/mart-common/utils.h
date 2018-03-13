#ifndef LIB_MART_COMMON_GUARD_UTILS_H
#define LIB_MART_COMMON_GUARD_UTILS_H
/**
 * utils.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: mixed set of utility functions
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <stdexcept>

/* Proprietary Library Includes */
#include "./cpp_std/type_traits.h"
#include "./enum/EnumHelpers.h"

/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */


namespace mart {

/* ######## narrowing ################################################ */

// narrow_cast(): a searchable way to do unchecked narrowing casts of values
template <class T, class U>
inline constexpr T narrow_cast(U u) noexcept
{
	static_assert(std::is_arithmetic<T>::value && std::is_arithmetic<U>::value, "Narrow cast can only be used for arithmetic types");
	return static_cast<T>(u);
}

struct narrowing_error : std::exception {};

namespace _impl_narrow {

template <class T, class U>
struct is_same_signedness
	: std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value> {};

template <class T, class U, bool SameSigndness= is_same_signedness<T,U>::value>
struct sign_check {
	//throws an narrowing error, if both parameters are of different signdness and one is negative
	static void check(T t,U u)
	{
		if ((t < T{}) != (u < U{})) {
			throw narrowing_error();
		}
	}
};

template <class T, class U>
struct sign_check<T,U,true> {
	//if both parameters are of same signess, we don't have to do anything
	static void check(T, U)
	{}
};

} // _impl_narrow

/**
 * Performs a narrowing cast and throws a narrowing_error exception if the source value can't be represented in the target type
 */
template <class T, class U>
inline T narrow(U u)
{
	T t = narrow_cast<T>(u);

	// this only does any work if T and U are of different signdness
	_impl_narrow::sign_check<T,U>::check(t, u);

	//check if roundtrip looses information
	if (static_cast<U>(t) != u) {
		throw narrowing_error();
	}

	return t;
}

/* ######## container ################################################ */

/**
 * creates a container and reserves the given amount of space.
 * So instead of
 *
 *	std::vector<int> vec;
 *	vec.reserve(100);
 *
 * you can write
 *
 *  auto vec = mart::make_with_capacity<std::vector<int>(100);
 */

template<class T>
T make_with_capacity(size_t i)
{
	T t;
	t.reserve(i);
	return t;
}
/* ######## math ################################################ */
template<class T>
T clamp(T val, T min_val, T max_val) {
	using std::min;
	using std::max;
	return min(max_val, max(min_val,val));
}


//Flag that is sometimes used as a template parameter for policy based design
enum class Synchonized {
	False,
	True
};

namespace detail {
template<class T>
constexpr bool type_is_one_of_impl(T*) {
	return false;
}

template<class T, class T1, class ... Ts>
constexpr bool type_is_one_of_impl(T* p, T1*, Ts* ... ps) {
	return std::is_same<T, T1>::value || type_is_one_of_impl(p, ps...);
}

}

template<class T, class T1, class ... Ts>
constexpr bool type_is_one_of() {
	return detail::type_is_one_of_impl((T*)nullptr, (T1*)nullptr, ((Ts*)nullptr) ...);
}

}//mart

#endif //LIB_MART_COMMON_GUARD_UTILS_H