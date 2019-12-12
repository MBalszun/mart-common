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
 * @brief:	Provides log Level type and helper functions
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <array>
#include <string_view>

/* Proprietary Library Includes */
#include "../enum/EnumHelpers.h"

/* Project Includes */
#include "MartLogFWD.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace log {

// TODO: rename to LogLvl
enum class Level {
	ERROR  = MART_LOG_LOG_LVL_ERROR,
	Error  = MART_LOG_LOG_LVL_ERROR,
	STATUS = MART_LOG_LOG_LVL_STATUS,
	Status = MART_LOG_LOG_LVL_STATUS,
	DEBUG  = MART_LOG_LOG_LVL_DEBUG,
	Debug  = MART_LOG_LOG_LVL_DEBUG,
	TRACE  = MART_LOG_LOG_LVL_TRACE,
	Trace  = MART_LOG_LOG_LVL_TRACE
};

constexpr Level defaultLogLevel = Level::Status;

// clang-format off
constexpr inline std::string_view to_string_view( Level lvl ) noexcept
{
	constexpr std::array<std::string_view, 4> names{ {
			std::string_view {"ERROR"},
			std::string_view {"STATUS"},
			std::string_view {"DEBUG"},
			std::string_view {"TRACE"}
		} };
	return names[mart::toUType(lvl)];
}

[[deprecated("toStringView is deprecated. Please use to_string_view")]] constexpr inline std::string_view toStringView( Level lvl ) noexcept
{
	return to_string_view(lvl);
}

// clang-format on

} // namespace log
} // namespace mart

#endif
