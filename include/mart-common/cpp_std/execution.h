#ifndef LIB_MART_COMMON_GUARD_CPP_STD_EXECUTION_H
#define LIB_MART_COMMON_GUARD_CPP_STD_EXECUTION_H
/**
 * execution.h (mart-common/cpp_std)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides replacements for standard library symbols in <execution>
 *			which are not yet provided by current c++ toolcahin
 *
 */

#if __has_include( <version>)
#include <version>
#else
#include <ciso646>
#endif

#ifndef MART_COMMON_STDLIB_HAS_PARALLEL_ALGORITHMS

#if defined( __cpp_lib_execution ) && defined( __cpp_lib_parallel_algorithm )
#define MART_COMMON_STDLIB_HAS_PARALLEL_ALGORITHMS 1
#else
#define MART_COMMON_STDLIB_HAS_PARALLEL_ALGORITHMS 0
#endif

#endif

#if MART_COMMON_STDLIB_HAS_PARALLEL_ALGORITHMS
#include <execution>
namespace mart {

template<class T>
inline constexpr bool is_execution_policy_v = std::is_execution_policy_v<T>;

namespace execution {

using std::execution::parallel_policy;
using std::execution::parallel_unsequenced_policy;
using std::execution::sequenced_policy;

constexpr const sequenced_policy&            seq        = std::execution::seq;
constexpr const parallel_policy&             par        = std::execution::par;
constexpr const parallel_unsequenced_policy& par_unsequ = std::execution::par_unseq;

} // namespace execution
} // namespace mart
#else
namespace mart {
namespace execution {

// clang-format off
class sequenced_policy {};
class parallel_policy {};
class parallel_unsequenced_policy {};
// clang-format on

inline constexpr sequenced_policy            seq {};
inline constexpr parallel_policy             par {};
inline constexpr parallel_unsequenced_policy par_unseq {};

} // namespace execution

// clang-format off
template<class T>
inline constexpr bool is_execution_policy_v =
			std::is_same_v<T, execution::sequenced_policy>
		||	std::is_same_v<T, execution::parallel_policy>
		||	std::is_same_v<T, execution::parallel_unsequenced_policy>;
// clang-format on

} // namespace mart

#endif

#endif
