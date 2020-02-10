#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_UNBLOCK_EXCPETION_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_UNBLOCK_EXCPETION_H
/**
 * UnblockException.h (mart-common/experimental/mt)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides exception type that is thrown if blocking
 *			read was canceled.
 *
 */

#include <stdexcept>

namespace mart {
namespace experimental {
namespace mt {

struct [[deprecated]] Canceled : std::exception{

};
} // namespace mt
} // namespace experimental
} // namespace mart

#endif
