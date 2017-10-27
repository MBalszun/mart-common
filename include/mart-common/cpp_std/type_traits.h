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
#include <algorithm>


namespace mart {

template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B, T>::type;

template< class T >
using remove_extent_t = typename std::remove_extent<T>::type;

template< bool B, class T, class F >
using conditional_t = typename std::conditional<B, T, F>::type;

template< class T >
using underlying_type_t = typename std::underlying_type<T>::type;

template< class... T >
using common_type_t = typename std::common_type<T...>::type;

template< class T >
using remove_reference_t = typename std::remove_reference<T>::type;

/*####### signed/unsigned ##################*/

template< class T >
using make_signed_t = typename  std::make_signed<T>::type;

template< class T >
using make_unsigned_t = typename std::make_unsigned<T>::type;


/*####### remove cv-qualifiers ##################*/
template< class T >
using remove_cv_t = typename std::remove_cv<T>::type;

template< class T >
using remove_const_t = typename std::remove_const<T>::type;

template< class T >
using remove_volatile_t = typename std::remove_volatile<T>::type;


/*####### add cv-qualifiers ##################*/
template< class T >
using add_cv_t = typename std::add_cv<T>::type;

template< class T >
using add_const_t = typename std::add_const<T>::type;

template< class T >
using add_volatile_t = typename std::add_volatile<T>::type;



template< class T >
using decay_t = typename std::decay<T>::type;

// from http://en.cppreference.com/w/cpp/types/conjunction
template<class...> struct conjunction : std::true_type {};
template<class B1> struct conjunction<B1> : B1 {};
template<class B1, class... Bn>
struct conjunction<B1, Bn...>
	: conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

// from http://en.cppreference.com/w/cpp/types/disjunction
template<class...> struct disjunction : std::false_type {};
template<class B1> struct disjunction<B1> : B1 {};
template<class B1, class... Bn>
struct disjunction<B1, Bn...>
	: conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

// from http://en.cppreference.com/w/cpp/types/aligned_union
template <std::size_t Len, class... Types>
struct aligned_union {
	//std::max is not constexpr in c++11 :(
	static constexpr size_t _max(size_t l) {
		return l;
	}
	static constexpr size_t _max(size_t l, size_t r) {
		return l < r ? r : l;
	}
	template<class ... ARGS>
	static constexpr size_t _max(size_t s1, size_t s2, size_t s3, ARGS... s) {
		return _max(_max(_max(s1, s2), s3), s ...);
	}

	static constexpr std::size_t alignment_value = _max( alignof(Types)... );

	struct type {
		alignas(8) char _s[_max(Len, sizeof(Types)... )];
	};
};

}
#endif
