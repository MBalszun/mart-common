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

#include <im_str/im_str.hpp>

namespace mart {
namespace log {

inline void defaultInit( Level logLvl = Level::Debug, mba::im_zstr name_tag = "main" )
{
	Logger& logger = Logger::initDefaultLogger( LoggerConf_t{name_tag, logLvl} );
	logger.addSink( log::makeSink( log::StdOutLogConfig_t{Level::Debug} ) );
}

inline void defaultInit( mba::im_zstr file_tag, Level logLvl = Level::Debug, mba::im_zstr name_tag = "main" )
{
	Logger& logger = Logger::initDefaultLogger( LoggerConf_t{name_tag, logLvl} );
	logger.addSink( log::makeSink( log::FileLogConfig_t{file_tag, Level::Trace} ) );
}

using LOG_LVL = Level;

} // namespace log
} // namespace mart

#endif