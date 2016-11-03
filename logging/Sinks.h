/*
 * Sinks.h
 *
 *  Created on: Nov 12, 2015
 *      Author: balszun
 */

#ifndef MART_COMMON_LOGGING_SINKS_H_
#define MART_COMMON_LOGGING_SINKS_H_

#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <fstream>
#include <iostream>
#include <memory>

/* Proprietary Library Includes */
#include <mart-common/ConstString.h>
#include <mart-common/StringView.h>

/* Project Includes */
#include "ILogSink.h"

namespace mart {
namespace log {

/*###### File log ######*/

class FileLog : public ILogSink {
	std::ofstream	 file;
	mart::ConstString _fileName;
	void _writeToLogImpl( mart::StringView msg ) override { file << msg; }
	void _flush() override { file.flush(); }

public:
	FileLog( mart::ConstString name, LOG_LVL lvl = LOG_LVL::TRACE )
		: ILogSink( lvl )
		, file( name.to_string() )
		, _fileName( name ){};

	mart::ConstString getName() const override { return _fileName; }
};

struct FileLogConfig_t {
	mart::ConstString fileName;
	LOG_LVL			  maxLogLvl;
};

inline std::shared_ptr<ILogSink> makeSink( const FileLogConfig_t& cfg )
{
	return std::make_shared<FileLog>( cfg.fileName, cfg.maxLogLvl );
}

/*###### StdOutLog ######*/

class StdOutLog : public ILogSink {
	StdOutLog()
	{
		// Most probably, stdout will be used in multiple different threads, so default to threadsafe mode
		this->enableThreadSafeMode( true );
	};
	void _writeToLogImpl( mart::StringView msg ) override { std::cout << msg; }
	void _flush() override { std::cout.flush(); }

public:
	static std::shared_ptr<StdOutLog> getInstance()
	{
		// make_shared not possible due to private constructor
		const static auto instance = std::shared_ptr<StdOutLog>( new StdOutLog() );
		return instance;
	}
	mart::ConstString getName() const override { return mart::ConstString{"COUT"}; }
};

struct StdOutLogConfig_t {
	LOG_LVL maxLogLvl;
};

inline std::shared_ptr<ILogSink> makeSink( const StdOutLogConfig_t& cfg )
{
	auto tmp	= StdOutLog::getInstance();
	tmp->maxlvl = cfg.maxLogLvl;
	return tmp;
}
}
}

#endif