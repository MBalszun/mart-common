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
#include <im_str/im_str.hpp>

/* Project Includes */
#include "ILogSink.h"
#include "SinkConfigs.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace log {

/*###### File log ######*/

class FileLog final : public ILogSink {
	std::ofstream _file;
	mba::im_zstr  _fileName;

	void _do_writeToLogImpl( std::string_view msg ) override { _file << msg; }
	void _do_flush() override { _file.flush(); }

public:
	FileLog( mba::im_zstr name, Level lvl = Level::TRACE )
		: ILogSink( lvl )
		, _file( std::string( std::string_view( name ) ) )
		, _fileName( name ){};

	mba::im_zstr getName() const override { return _fileName; }
};

inline std::shared_ptr<ILogSink> makeSink( const FileLogConfig_t& cfg )
{
	return std::make_shared<FileLog>( cfg.fileName, cfg.maxLogLvl );
}

/*###### StdOutLog ######*/

class StdOutLog final : public ILogSink {
	StdOutLog()
	{
		// Most probably, stdout will be used in multiple different threads, so default to threadsafe mode
		this->enableThreadSafeMode( true );
	};

	void _do_writeToLogImpl( std::string_view msg ) override { std::cout << msg; }
	void _do_flush() override { std::cout.flush(); }

public:
	static std::shared_ptr<StdOutLog> getInstance()
	{
		// make_shared not possible due to private constructor
		const static auto instance = std::shared_ptr<StdOutLog>( new StdOutLog() );
		return instance;
	}
	mba::im_zstr getName() const override { return mba::im_zstr{"COUT"}; }
};

inline std::shared_ptr<ILogSink> makeSink( const StdOutLogConfig_t& cfg )
{
	auto tmp    = StdOutLog::getInstance();
	tmp->maxlvl = cfg.maxLogLvl;
	return tmp;
}
} // namespace log
} // namespace mart

#endif