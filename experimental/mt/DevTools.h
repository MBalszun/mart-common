#pragma once

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