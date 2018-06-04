#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_MOD_SEQUENCE_OPS_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_MOD_SEQUENCE_OPS_H
/**
 * mod-sequence-ops.h (mart-common/algorithms)
 *
 * Copyright (R) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides wrappers around standard library modifying sequence operations
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
/* Proprietary Library Includes */
/* Standard Library Includes */
#include <algorithm>
#include <iterator>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

// Will be undefined at end of file
#define MART_COMMON_ALL( rng ) std::begin( rng ), std::end( rng )

namespace mart {

} // namespace mart

#undef MART_COMMON_ALL

#endif
