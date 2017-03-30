#ifndef LIB_MART_COMMON_GUARD_LOGGING_TYPES_H
#define LIB_MART_COMMON_GUARD_LOGGING_TYPES_H
/**
 * types.h (mart-common/logging)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides LOG_LVL type and helper functions
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <array>

/* Proprietary Library Includes */
#include "../ConstString.h"
#include "../StringView.h"
#include "../utils.h"

/* Project Includes */
#include "MartLogFWD.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace log {

//TODO: rename to LogLvl
enum class LOG_LVL {
	ERROR = MART_LOG_LOG_LVL_ERROR,
	STATUS = MART_LOG_LOG_LVL_STATUS,
	DEBUG = MART_LOG_LOG_LVL_DEBUG,
	TRACE = MART_LOG_LOG_LVL_TRACE
};

constexpr LOG_LVL defaultLogLevel = LOG_LVL::STATUS;

//clang-format off
inline mart::StringView toStringView(LOG_LVL lvl)
{
	static constexpr std::array<mart::StringView, 4> names{ {
			mart::StringView{ "ERROR" },
			mart::StringView{ "STATUS" },
			mart::StringView{ "DEBUG" },
			mart::StringView{ "TRACE" }
		} };
	return names[mart::toUType(lvl)];
}

inline const std::string& toString(LOG_LVL lvl)
{
	static const std::array<std::string, 4> names{ {
			std::string{ "ERROR" },
			std::string{ "STATUS" },
			std::string{ "DEBUG" },
			std::string{ "TRACE" }
		} };
	return names[mart::toUType(lvl)];
}
//clang-format on

}
}

#endif
