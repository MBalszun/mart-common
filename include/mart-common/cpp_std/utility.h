#ifndef LIB_MART_COMMON_GUARD_CPP_STD_UTILITY_H
#define LIB_MART_COMMON_GUARD_CPP_STD_UTILITY_H

#include <utility>


namespace mart {
	//from cppreference.com
	template<class T, class U = T>
	T exchange(T& obj, U&& new_value)
	{
		T old_value = std::move(obj);
		obj = std::forward<U>(new_value);
		return old_value;
	}
}
#endif


