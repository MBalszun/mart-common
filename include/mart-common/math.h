#ifndef LIB_MART_COMMON_GUARD_MATH_H
#define LIB_MART_COMMON_GUARD_MATH_H
/**
 * math.h (mart-common)
 *
 * Copyright (C) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: math utilities
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */

/* Proprietary Library Includes */

/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

template<class T>
auto square( const T& value )
{
	return value * value;
}

} // namespace mart
#endif
