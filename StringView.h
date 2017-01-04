#ifndef MART_LIB_COMMON_STRING_VIEW_H
#define MART_LIB_COMMON_STRING_VIEW_H
#pragma once

#include "./experimental/StringView.h"

#include "algorithm.h"
#include "cpp_std/type_traits.h"

#include <cassert>

namespace mart {

namespace details_to_integral {
//Core logic.
//Assuming number starts at first character and string is supposed to be parsed until first non-digit
template<class T>
T core(mart::StringView str)
{
	T tmp = 0;
	for (auto c : str) {
		int d = c - '0';
		if (d < 0 || 9 < d ) {
			break;
		}
		if (tmp >= std::numeric_limits<T>::max() / 16) { // quick check against simple constant
			if ( tmp > (std::numeric_limits<T>::max() - d) / 10 ) {
				throw std::out_of_range("String representing an integral (\"" + str.to_string() + "\") overflows overflows type " + typeid(T).name());
			}
		}
		tmp = tmp * 10 + d;

	}
	return tmp;
}

//for unsigned types
template<class T>
auto base(const mart::StringView str) -> mart::enable_if_t<std::is_unsigned<T>::value, T>
{
	assert(str.size() > 0);
	if (str[0] == '+') {
		return details_to_integral::core<T>(str.substr(1));
	} else {
		return details_to_integral::core<T>(str);
	}
}

//for signed types
template<class T>
auto base(const mart::StringView str) -> mart::enable_if_t<std::is_signed<T>::value, T>
{
	assert(str.size() > 0);
	switch(str[0]) {
		case '-' : return -details_to_integral::core<T>(str.substr(1));
		case '+' : return  details_to_integral::core<T>(str.substr(1));
		default  : return  details_to_integral::core<T>(str);
	}
}
}

template<class T = int>
T to_integral(const mart::StringView str) {
	if (str.size() == 0) {
		return T{};
	}
	return details_to_integral::base<T>(str);
}

//minimal implementation for sanitized strings that only contain digits (no negative numbers)
template<class T>
T to_integral_unsafe(mart::StringView str)
{
	return mart::accumulate(str, T{}, [](T sum, char c) { return sum * 10 + c - '0'; });
}

}

#endif
