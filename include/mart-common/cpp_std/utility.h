#ifndef LIB_MART_COMMON_GUARD_CPP_STD_UTILITY_H
#define LIB_MART_COMMON_GUARD_CPP_STD_UTILITY_H
/**
 * utility.h (mart-common/cpp_std)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides replacements for standard library symbols in <utility>
 *			which are not yet provided by current c++ toolcahin
 *
 */

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

	template<class T, T... Vals>
	struct integer_sequence {
		using value_type = T;
		static constexpr std::size_t size() { return sizeof...(Vals); };
	};

	template<std::size_t ... Vals>
	using index_sequence = integer_sequence<std::size_t, Vals ...>;

	template <std::size_t N, size_t ... Vals>
	struct make_index_sequence_impl {
		using type = typename make_index_sequence_impl<N - 1, N, Vals ...>::type;
	};

	template < std::size_t ... Vals>
	struct make_index_sequence_impl<0, Vals...> {
		using type = index_sequence<0, Vals ...>;
	};

	template < std::size_t N>
	using make_index_sequence = typename make_index_sequence_impl<N-1>::type;

}

#endif
