#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_RANGES_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_RANGES_H
/**
 * ranges.h (mart-common/experimetnal)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	extensions to the facilities in the main ranges.h header that need more development
 *
 */

#include "../ranges.h"

namespace mart {
namespace experimental {

template<class It>
struct range {
	It _begin;
	It _end;
	constexpr It begin() const { return _begin; }
	constexpr It end() const { return _end; }
};

template<class R>
auto view_reversed(R&& r) -> range<std::reverse_iterator<decltype(r.begin())>> {
	using Rit = std::reverse_iterator<decltype(r.begin())>;
	return{ Rit{ r.end() },Rit{ r.begin() } };
}

}
}

#endif