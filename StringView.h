#ifndef MART_LIB_COMMON_STRING_VIEW_H
#define MART_LIB_COMMON_STRING_VIEW_H
#pragma once

#include "./experimental/StringView.h"


namespace mart {

	template<class T = int>
	T to_integral(mart::StringView str)
	{
		if (str.size() == 0) {
			return T{};
		}

		typename std::make_unsigned<T>::type tmp = 0;
		bool neg = str[0] == '-';
		if (neg || str[0] == '+') {
			str = str.substr(1);
		}
		for (auto c : str) {
			auto d = c - '0';
			if ('0'<=c && c <='9') {
				tmp *= 10;
				tmp += d;
			} else {
				break;
			}
		}
		return neg ? - static_cast<T>(tmp) : static_cast<T>(tmp);
	}
}

#endif
