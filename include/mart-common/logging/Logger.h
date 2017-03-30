#ifndef LIB_MART_COMMON_GUARD_LOGGING_LOGGER_H
#define LIB_MART_COMMON_GUARD_LOGGING_LOGGER_H

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

/* Proprietary Library Includes */
#include <mart-common/ConstString.h>
#include <mart-common/MartTime.h>
#include <mart-common/StringView.h>
#include <mart-common/experimental/CopyableAtomic.h>
#include <mart-common/utils.h>

/* Project Includes */
#include "MartLogFWD.h"
#include "types.h"
#include "default_formatter.h"
#include "ILogSink.h"

namespace mart {
namespace log {

/*########### LOGGER ##########*/

class ILogSink;

/**
 * @brief Logger class
 *
 * CAN'T be used from multiple threads
 * Can write to multiple logs
 */
struct LoggerConf_t {
	mart::ConstString moduleName;
	LOG_LVL			  logLvl;
};

class Logger {
	enum class AddNewline { No, Yes };

public:
	/* ### CTORS ### */
	/**
	 * Normal constructor only requiring a module name
	 * @param moduleName Name of the module by which the logger is used (this will be printed at the beginning of each line in the log)
	 */
	Logger( const mart::StringView moduleName, LOG_LVL logLvl = defaultLogLevel )
		: _startTime{mart::now()}
		, _currentLogLevel{logLvl}
		, _enabled{true}
		, _sinks{}
		, _loggingName( _createLoggingName( moduleName ) )
	{
	}

	/**
	 * Constructor which takes a default sink.
	 * @param moduleName Name of the module by which the logger is used (this will be printed at the beginning of each line in the log)
	 * @param sink pointer to sink, where the output is written to MUST NOT BE NULL!
	 */
	Logger( mart::StringView moduleName, std::shared_ptr<ILogSink> sink, LOG_LVL logLvl = defaultLogLevel )
		: Logger( moduleName, logLvl )
	{
		addSink( sink );
	}

	Logger( const LoggerConf_t& cfg )
		: Logger( cfg.moduleName, cfg.logLvl )
	{
	}

	/**
	 * Constructs logger from parent logger.
	 *
	 * Copies all settings and adds submodule name
	 * @param moduleName
	 * @param other
	 */
	Logger( const mart::StringView subModuleName, const Logger& other )
		: Logger(other)
	{
		_loggingName = _createLoggingName(subModuleName, other._loggingName);
	}

	Logger(Logger&& other) noexcept = default;
	Logger(const Logger& other) = default;
	Logger& operator=(Logger&& other) noexcept {//= default; doesn't work for some reason
		_startTime			= other._startTime;
		_currentLogLevel	= other._currentLogLevel;
		_enabled			= other._enabled;
		_sinks				= std::move(other._sinks);
		_loggingName		= std::move(other._loggingName);
		_spacer				= std::move(other._spacer);
		return *this;
	}

	Logger& operator=(const Logger& other) = default;

	/* ### Statics ### */
	//NOTE: This is NOT a singleton
	/**
	 * Only has effect if called before first call to getDefaultLogger()
	 */
	static Logger& initDefaultLogger( const LoggerConf_t& conf )
	{
		static Logger instance( conf );
		return instance;
	}

	static Logger& initDefaultLogger( const LoggerConf_t& conf, const std::vector<std::shared_ptr<ILogSink>>& sinks )
	{
		std::cout << "Initializing default logger\n";
		Logger& lref = initDefaultLogger( conf );
		for ( auto& sink : sinks ) {
			lref.addSink( sink );
			std::cout << "Added sink: " << sink->getName() << '\n';
		}
		return lref;
	}

	static Logger& getDefaultLogger()
	{
		static Logger& instance = initDefaultLogger( {} );
		return instance;
	}

	/* ####### log interface #######*/
	template <class... ARGS>
	void log( LOG_LVL lvl, ARGS&&... args )
	{
		//Bail out of formatting and stuff early, if message should not be logged in the first place
		if ( !_shouldBeLogged( lvl ) )
			return;

		_fillBuffer( lvl, AddNewline::Yes, std::forward<ARGS>( args )... );
		_writeBufferToSinks( lvl );
	}

	/* ### Change logging behavior #### */
	/**
	 * Gets the highest (TRACE > ERROR) log level with which messages are currently written to log
	 * @return current log level
	 */
	LOG_LVL getLogLevel() const
	{
		return _currentLogLevel.load( std::memory_order_relaxed );
	}
	void setLogLevel( LOG_LVL lvl )
	{
		_currentLogLevel.store( lvl, std::memory_order_relaxed );
	}

	/**
	 * Enables or disables logging, without changing log level - currently only way to prevent logging of errors
	 * @param enable
	 */
	void enable( bool enable = true )
	{
		_enabled.store( enable, std::memory_order_relaxed );
	}
	void disable()
	{
		enable( false );
	}
	bool isEnabled() const
	{
		return _enabled;
	}

	void setName( mart::StringView name )
	{
		_loggingName = _createLoggingName( name );
	}

	/* ### Change sinks ###*/
	void addSink( std::shared_ptr<ILogSink> sink )
	{
		if ( sink != nullptr ) {
			_sinks.emplace_back( sink );
		}
	}
	void clearSinks()
	{
		_sinks.clear();
	}
	std::vector<std::shared_ptr<ILogSink>> getSinks() const
	{
		return _sinks;
	}

	/*### functions related to indendation level (mostly relevant for function call stack tracing) ###*/
	/**
	 * Increases indentation level for all following log messages created from this logger by two
	 */
	void bumpIndentLevel()
	{
		_spacer.append( "  " );
	}

	/**
	 * Decreases indentation level for all following log messages created from this logger by two
	 */
	void removeIndentLevel()
	{
		_spacer.pop_back();
		_spacer.pop_back();
	}

	struct IndentLevelGuard {
		Logger* _log;
		explicit IndentLevelGuard( Logger& logger )
			: _log( &logger )
		{
			logger.bumpIndentLevel();
		}
		IndentLevelGuard( IndentLevelGuard&& other ) noexcept
			: _log( other._log )
		{
			other._log = nullptr;
		}
		IndentLevelGuard( const IndentLevelGuard& ) = delete;

		~IndentLevelGuard()
		{
			if ( _log )
				_log->removeIndentLevel();
		}
	};

	/**
	 * Same as bumpIdentLevel, but removes indentation level as soon as the returned guard object is destroyed
	 * This can be used to couple the log indentation level to a scope in an RAII manner
	 * @return Guard Object to whose lifetime the indentation level is coupled
	 */
	IndentLevelGuard bumpIdentLevelGuarded()
	{
		return IndentLevelGuard( *this );
	}

private:
	/*### Variables controlling logging behavior ###*/
	mart::copter_time_point		  _startTime;
	mart::CopyableAtomic<LOG_LVL> _currentLogLevel;
	mart::CopyableAtomic<bool>	  _enabled;

	std::vector<std::shared_ptr<ILogSink>> _sinks;

	/*### Cached parts of logged message ### */
	mart::ConstString _loggingName; //This is what can be grepped for in the logfile
	std::string		  _spacer;

	static std::ostringstream& _sbuffer()
	{
		thread_local std::ostringstream stream;
		return stream;
	}

	// checks if a message  with priority <lvl> should be logged or not
	// Note, this is thread safe, but doesn't synchronize with e.g. disable()
	bool _shouldBeLogged( LOG_LVL lvl )
	{
		//TODO: look at log Level of attached logger?
		return _enabled.load( std::memory_order_relaxed )
			   && ( lvl <= _currentLogLevel.load( std::memory_order_relaxed ) );
	}

	static mart::ConstString _createLoggingName( const StringView moduleName,
												 const StringView parentName = EmptyStringView )
	{
		return mart::concat( parentName, "[", moduleName, "]" );
	}

	//Function where the actual message gets composed
	template <class... ARGS>
	void _fillBuffer( LOG_LVL lvl, AddNewline newLine, ARGS&&... args )
	{
		//line prefix
		formatForLog( _sbuffer(), lvl, " - At ", std::setw(7) , passedTime<milliseconds>( _startTime ), " - ", _loggingName, ": " );

		//Add thread Id and spacer in trace mode
		if ( _currentLogLevel == LOG_LVL::TRACE ) {
			ostream_flag_saver _( _sbuffer() );
			formatForLog( _sbuffer(), "[ThreadID: ", std::this_thread::get_id(), "]: ", _spacer );
		}

		//write actual message
		formatForLog( _sbuffer(), args... );

		//Append new line if requested
		if ( newLine == AddNewline::Yes ) {
			_sbuffer() << '\n';
		}
	}

	//write contents to all registered log sinks and reset buffer
	void _writeBufferToSinks( LOG_LVL lvl )
	{
		auto text = _sbuffer().str();
		_sbuffer().str( std::string{} );
		for ( const auto& se : _sinks ) {
			se->writeToLog( text, lvl );
		}
	}
};

//separate function, that can be forward declared
inline Logger& getDefaultLogger()
{
	return Logger::getDefaultLogger();
}




} /*namespace log*/
} /*namespace mart*/

#endif /* SRC_UTILS_MARTLOG_H_ */
