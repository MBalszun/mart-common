#ifndef LIB_MART_COMMON_GUARD_LOGGING_TYPES_H
#define LIB_MART_COMMON_GUARD_LOGGING_TYPES_H

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <array>

/* Project Includes */
#include "../StringView.h"
#include "../utils.h"

#include "MartLogFWD.h"

namespace mart {
namespace log {

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
