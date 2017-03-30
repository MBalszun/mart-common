#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_DEV_TOOLS_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_DEV_TOOLS_H
/**
 * DevTools.h (mart-common/experimental)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides functions that are only relevant for testing and development
 *
 */

#include <chrono>

namespace mart {
namespace experimental {
namespace dev {

template<class F>
std::chrono::nanoseconds execTimed(F&& f)
{
	using tclock = std::chrono::steady_clock;
	auto start = tclock::now();
	f();
	auto end = tclock::now();
	return end - start;
}

}
}
}

#endif
