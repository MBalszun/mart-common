#ifndef LIB_MART_COMMON_GUARD_LOGGING_ILOG_SINK_H
#define LIB_MART_COMMON_GUARD_LOGGING_ILOG_SINK_H
/**
 * ILogSink.h (mart-common/logging)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides Interface for logging sinks
 *
 */


#include "types.h"

#include <atomic>
#include <mutex>
#include <string_view>
#include <im_str/im_str.hpp>

namespace mart {

class ConstString;

namespace log {
/**
 * Interface which log sinks must implement in order to be compatible with the logger
 */
class ILogSink {
public:
	ILogSink( Level lvl = Level::TRACE )
		: maxlvl( lvl )
	{
	}
	virtual ~ILogSink() = default;

	void enableThreadSafeMode( bool enable ) { _threadSafe = enable; }

	bool isInThreadSafeMode() const { return _threadSafe; }

	void writeToLog( std::string_view msg, Level lvl )
	{
		// only log messages with lower or equal log level (higher importance) than maxlvl
		if( lvl > maxlvl ) {
			return;
		}

		if( _threadSafe ) {
			std::lock_guard<std::mutex> ul( _mux );
			_writeToLogImpl( msg );
			if( lvl <= Level::STATUS ) {
				_flush();
			}
		} else {
			_writeToLogImpl( msg );
			if( lvl <= Level::STATUS ) {
				_flush();
			}
		}
	}

	void flush()
	{
		if( _threadSafe ) {
			std::lock_guard<std::mutex> ul( _mux );
			_flush();
		} else {
			_flush();
		}
	};

	virtual mba::im_zstr getName() const = 0;

	// Maximum level up to which messages are actually written to this sink
	std::atomic<Level> maxlvl;

private:
	std::mutex		  _mux;
	std::atomic<bool> _threadSafe{true};

	/// actual logging function that has to be implemented by sinks
	virtual void _writeToLogImpl( std::string_view msg ) = 0;
	virtual void _flush()								 = 0;
};
}
}

#endif