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
#include <atomic>

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

template<class T = void>
struct Counter {
	static std::atomic<int> defaultConstructionCnt;
	static std::atomic<int> CopyConstructionCnt;
	static std::atomic<int> MoveConstructionCnt;
	static std::atomic<int> CopyAssignmentCnt;
	static std::atomic<int> MoveAssignmentCnt;
	static std::atomic<int> DestructionCnt;
protected:

	Counter() {
		defaultConstructionCnt.fetch_add(1, std::memory_order_relaxed);
	}
	Counter(const Counter&) {
		CopyConstructionCnt.fetch_add(1, std::memory_order_relaxed);
	}
	Counter(Counter&&) {
		MoveConstructionCnt.fetch_add(1, std::memory_order_relaxed);
	}
	Counter& operator=(const Counter&) {
		CopyAssignmentCnt.fetch_add(1, std::memory_order_relaxed);
		return *this;
	}
	Counter& operator=(Counter&&) {
		MoveAssignmentCnt.fetch_add(1, std::memory_order_relaxed);
		return *this;
	}
	~Counter() {
		DestructionCnt.fetch_add(1, std::memory_order_relaxed);
	}
};

template<class T> std::atomic<int> Counter<T>::defaultConstructionCnt;
template<class T> std::atomic<int> Counter<T>::CopyConstructionCnt;
template<class T> std::atomic<int> Counter<T>::MoveConstructionCnt;
template<class T> std::atomic<int> Counter<T>::CopyAssignmentCnt;
template<class T> std::atomic<int> Counter<T>::MoveAssignmentCnt;
template<class T> std::atomic<int> Counter<T>::DestructionCnt;

}
}
}

#endif
