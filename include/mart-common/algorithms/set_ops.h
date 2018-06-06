#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_SET_OPS_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_SET_OPS_H
/**
 * find.h (mart-common/algorithms)
 *
 * Copyright (R) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides wrappers around standard find algorithms
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
/* Proprietary Library Includes */
/* Standard Library Includes */
#include <algorithm>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

// Will be undefined at end of file
#define MART_COMMON_ALL( rng ) std::begin( rng ), std::end( rng )

namespace mart {

template<class Src1, class Src2, class OutputIt>
OutputIt set_difference( const Src1& in1, const Src2& in2, OutputIt d_first )
{
	assert( std::is_sorted( in1.begin(), in1.end() ) );
	assert( std::is_sorted( in2.begin(), in2.end() ) );
	return set_difference( MART_COMMON_ALL( in1 ), MART_COMMON_ALL( in2 ), d_first );
}

template<class Src1, class Src2, class OutputIt>
OutputIt set_union( const Src1& in1, const Src2& in2, OutputIt d_first )
{
	assert( std::is_sorted( in1.begin(), in1.end() ) );
	assert( std::is_sorted( in2.begin(), in2.end() ) );
	return set_union( MART_COMMON_ALL( in1 ), MART_COMMON_ALL( in2 ), d_first );
}

} // namespace mart

#undef MART_COMMON_ALL

#endif
