#pragma once

#include <type_traits>


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

}