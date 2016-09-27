#pragma once
#include <memory>
#include "type_traits.h"

namespace mart {

//#### make unique #####
template<typename T, typename ...Args>
inline std::unique_ptr<T> make_unique(Args&& ...args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
inline auto make_unique(std::size_t size) -> mart::enable_if_t<
														std::is_array<T>::value && std::extent<T>::value == 0,
														std::unique_ptr<T>
													>
{
	using Element_t = mart::remove_extent_t<T>;
	return std::unique_ptr<T>(new Element_t[size]());
}

}