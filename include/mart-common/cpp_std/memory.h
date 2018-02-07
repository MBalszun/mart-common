#ifndef LIB_MART_COMMON_GUARD_CPP_STD_MEMORY_H
#define LIB_MART_COMMON_GUARD_CPP_STD_MEMORY_H
/**
 * memory.h (mart-common/cpp_std)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides replacements for standard library symbols in <memory>
 *			which are not yet provided by current c++ toolcahin
 *
 */

#include <memory>
#include "type_traits.h"

namespace mart {

using std::make_unique;

}

#endif
