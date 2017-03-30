#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_DEV_TOOLS_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_DEV_TOOLS_H

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
