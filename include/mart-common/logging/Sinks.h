#ifndef LIB_MART_COMMON_GUARD_LOGGING_SINKS_H
#define LIB_MART_COMMON_GUARD_LOGGING_SINKS_H
/**
 * Sinks.h (mart-common/mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	library provided log sinks that implement of the ILogSink interface
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <fstream>
#include <iostream>
#include <memory>

/* Proprietary Library Includes */
#include "../ConstString.h"

/* Project Includes */
#include "ILogSink.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace log {

/*###### File log ######*/

class FileLog : public ILogSink {
	std::ofstream		_file;
	mart::ConstString	_fileName;
	void _writeToLogImpl( std::string_view msg ) override { _file << msg; }
	void _flush() override { _file.flush(); }

public:
	FileLog( mart::ConstString name, Level lvl = Level::TRACE )
		: ILogSink( lvl )
		, _file( name.to_string() )
		, _fileName( name ){};

	mart::ConstString getName() const override { return _fileName; }
};

struct FileLogConfig_t {
	mart::ConstString fileName;
	Level			  maxLogLvl;
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
	void _writeToLogImpl( std::string_view msg ) override { std::cout << msg; }
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
	Level maxLogLvl;
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