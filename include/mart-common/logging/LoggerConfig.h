#ifndef LIB_MART_COMMON_GUARD_LOGGER_CONFIG_H
#define LIB_MART_COMMON_GUARD_LOGGER_CONFIG_H
/**
 * LoggerConfig.h (mart-common/Logging)
 *
 * Copyright (C) 2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides the actual logger class
 *
 */

#include "types.h"

#include "../ConstString.h"

namespace mart {
namespace log {

// TODO: move to separate file
struct LoggerConf_t {
	mart::ConstString moduleName;
	Level             logLvl = defaultLogLevel;
};

} // namespace log
} // namespace mart

#endif