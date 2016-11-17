#pragma once

#include "logging/Logger.h"
#include "logging/Sinks.h"

namespace mart {
namespace log {

inline void defaultInit(LOG_LVL logLvl = LOG_LVL::DEBUG, mart::StringView name_tag = mart::StringView("main"))
{
	Logger& logger = Logger::initDefaultLogger(LoggerConf_t{ mart::ConstString(name_tag) , logLvl });
	logger.addSink(log::makeSink(log::StdOutLogConfig_t{ LOG_LVL::DEBUG }) );
}

}
}