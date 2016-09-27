#pragma once

#include <type_traits>


namespace mart {

template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B, T>::type;

template< class T >
using remove_extent_t = typename std::remove_extent<T>::type;

}