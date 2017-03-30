#ifndef LIB_MART_COMMON_GUARD_MART_LOG_H
#define LIB_MART_COMMON_GUARD_MART_LOG_H
/**
 * MartLog.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: Provides simple logging facilities.
 */

#include "./logging/Logger.h"
#include "./logging/Sinks.h"

namespace mart {
namespace log {

inline void defaultInit(LOG_LVL logLvl = LOG_LVL::DEBUG, mart::StringView name_tag = mart::StringView("main"))
{
	Logger& logger = Logger::initDefaultLogger(LoggerConf_t{ mart::ConstString(name_tag) , logLvl });
	logger.addSink(log::makeSink(log::StdOutLogConfig_t{ LOG_LVL::DEBUG }) );
}

inline void defaultInit(mart::StringView file_tag, LOG_LVL logLvl = LOG_LVL::DEBUG, mart::StringView name_tag = mart::StringView("main"))
{
	Logger& logger = Logger::initDefaultLogger(LoggerConf_t{ mart::ConstString(name_tag) , logLvl });
	logger.addSink(log::makeSink(log::FileLogConfig_t{ mart::ConstString{file_tag},LOG_LVL::TRACE }));
}

}
}

#endif