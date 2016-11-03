#pragma once

#include <atomic>
#include <mutex>

#include "types.h"

namespace mart {

class StringView;
class ConstString;

namespace log {
/**
 * Interface which log sinks must implement in order to be compatible with the logger
 */
class ILogSink {
public:
	ILogSink( LOG_LVL lvl = LOG_LVL::TRACE )
		: maxlvl( lvl )
	{
	}
	virtual ~ILogSink() = default;

	void enableThreadSafeMode( bool enable ) { _threadSafe = enable; }

	bool isInThreadSafeMode() const { return _threadSafe; }

	void writeToLog( mart::StringView msg, LOG_LVL lvl )
	{
		// only log messages with lower or equal log level (higher importance) than maxlvl
		if( lvl > maxlvl ) {
			return;
		}

		if( _threadSafe ) {
			std::lock_guard<std::mutex> ul( _mux );
			_writeToLogImpl( msg );
			if( lvl <= LOG_LVL::ERROR ) {
				_flush();
			}
		} else {
			_writeToLogImpl( msg );
			if( lvl <= LOG_LVL::ERROR ) {
				_flush();
			}
		}

		// flush important messages directly
		if( lvl <= LOG_LVL::ERROR ) {
			flush();
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

	virtual mart::ConstString getName() const = 0;

	// Maximum level up to which messages are actually written to this sink
	std::atomic<LOG_LVL> maxlvl;

private:
	std::mutex		  _mux;
	std::atomic<bool> _threadSafe{true};

	/// actual logging function that has to be implemented by sinks
	virtual void _writeToLogImpl( mart::StringView msg ) = 0;
	virtual void _flush()								 = 0;
};
}
}