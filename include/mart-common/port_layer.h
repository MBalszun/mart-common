#ifndef LIB_MART_COMMON_GUARD_PORT_LAYER_H
#define LIB_MART_COMMON_GUARD_PORT_LAYER_H
/**
 * port_layer.h (mart-common)
 *
 * Copyright (C) 2019: Michael Balszun <michael.balszun@tum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: Portabgility utilities
 *
 */

// clang-format off
#ifdef _MSC_VER
	#define LIB_MART_COMMON_NO_INLINE __declspec( noinline )
#else
	#define LIB_MART_COMMON_NO_INLINE __attribute__((noinline))
#endif
// clang-format on

#endif