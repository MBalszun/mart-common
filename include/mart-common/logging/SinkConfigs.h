#ifndef LIB_MART_COMMON_GUARD_LOGGING_SINK_CONFIGS_H
#define LIB_MART_COMMON_GUARD_LOGGING_SINK_CONFIGS_H
/**
 * SinkConfigs.h (mart-common/mart-common)
 *
 * Copyright (C) 2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Configs for the library provided logging sinks
 *
 */

#include "types.h"

#include <im_str/im_str.hpp>
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace log {

struct FileLogConfig_t {
	mba::im_zstr fileName;
	Level        maxLogLvl;
};

struct StdOutLogConfig_t {
	Level maxLogLvl;
};

} // namespace log
} // namespace mart

#endif