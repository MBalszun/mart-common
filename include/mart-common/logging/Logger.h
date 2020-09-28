#ifndef LIB_MART_COMMON_GUARD_LOGGING_LOGGER_H
#define LIB_MART_COMMON_GUARD_LOGGING_LOGGER_H
/**
 * Logger.h (mart-common/Logging)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides the actual logger class
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <type_traits>
#include <vector>

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include <mutex>
#include <thread>

/* Proprietary Library Includes */
#include <im_str/im_str.hpp>

#include "../MartTime.h"
#include "../experimental/CopyableAtomic.h"
#include "../port_layer.h"
#include "../utils.h"

/* Project Includes */
#include "ILogSink.h"
#include "LoggerConfig.h"
#include "MartLogFWD.h"
#include "default_formatter.h"
#include "types.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace log {

/**
 * @brief Logger class
 *
 * CAN'T be used from multiple threads
 * Can write to multiple logs
 */

namespace detail {

template<class ARG, class = std::enable_if_t<std::is_convertible_v<ARG&&, std::string_view>>>
inline std::string_view forward_as_string_view_if_possible( ARG&& arg )
{
	return std::string_view( arg );
}

template<class ARG, class = std::enable_if_t<!std::is_convertible_v<ARG&&, std::string_view>>>
inline decltype( auto ) forward_as_string_view_if_possible( ARG&& args )
{
	return std::forward<ARG>( args );
}

static_assert( std::is_same_v<std::string_view, decltype( forward_as_string_view_if_possible( "Hello World" ) )> );

} // namespace detail

class Logger {
	enum class AddNewline { No, Yes };

public:
	/* ### CTORS ### */
	/**
	 * Normal constructor only requiring a module name
	 * @param moduleName Name of the module by which the logger is used (this will be printed at the beginning of each
	 * line in the log)
	 */
	Logger( const std::string_view moduleName, Level logLvl = defaultLogLevel )
		: _startTime{mart::now()}
		, _currentLogLevel{logLvl}
		, _enabled{true}
		, _sinks{}
		, _loggingName( _createLoggingName( moduleName ) )
	{
	}

	/**
	 * Constructor which takes a default sink.
	 * @param moduleName Name of the module by which the logger is used (this will be printed at the beginning of each
	 * line in the log)
	 * @param sink pointer to sink, where the output is written to MUST NOT BE NULL!
	 */
	Logger( const std::string_view moduleName, std::shared_ptr<ILogSink> sink, Level logLvl = defaultLogLevel )
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
	Logger( const std::string_view subModuleName, const Logger& other )
		: Logger( other )
	{
		_loggingName = _createLoggingName( subModuleName, other._loggingName );
	}

	Logger( const Logger& other )     = default;
	Logger( Logger&& other ) noexcept = default;
	Logger& operator=( const Logger& other ) = default;
	Logger& operator=( Logger&& other ) noexcept = default;

	Logger make_child( const std::string_view subModuleName ) const { return Logger( subModuleName, *this ); }

	/* ### Statics ### */
private:
	// NOTE: this whole function will ever only be called once and only from initDefaultLogger
	static Logger& _initDefaultLogger_impl( const LoggerConf_t& conf )
	{
#if MART_LOG_MAX_LOG_LVL == MART_LOG_LOG_LVL_TRACE
		if( conf.logLvl == mart::log::Level::Trace ) {
			std::cout << "[MartLog] Initializing default logger" << std::endl;
		}
#endif //
		static Logger instance( conf );
		return instance;
	}

public:
	// NOTE: This is NOT a singleton
	/**
	 * Only has effect if called before first call to getDefaultLogger()
	 */
	inline static Logger& initDefaultLogger( const LoggerConf_t& conf )
	{
		static Logger& instance = _initDefaultLogger_impl( conf );
		return instance;
	}

	static Logger& initDefaultLogger( const LoggerConf_t& conf, std::shared_ptr<ILogSink> sink )
	{
		Logger& lref = initDefaultLogger( conf );
		lref.addSink( std::move( sink ) );
		std::cout << "[MartLog] Added sink to default logger: " << sink->getName() << '\n';
		return lref;
	}

	static Logger& initDefaultLogger( const LoggerConf_t& conf, const std::vector<std::shared_ptr<ILogSink>>& sinks )
	{
		Logger& lref = initDefaultLogger( conf );
		for( auto& sink : sinks ) {
			lref.addSink( sink );
			std::cout << "[MartLog] Added sink to default logger: " << sink->getName() << '\n';
		}
		return lref;
	}

	static Logger& getDefaultLogger()
	{
		static Logger& instance = initDefaultLogger( {} );
		return instance;
	}

	/* ####### log interface #######*/
	template<class... ARGS>
	inline void log( Level lvl, ARGS&&... args )
	{
		// If message should not be logged in the first place
		// Bail out of formatting and other expensive stuff early
		if( !_shouldBeLogged( lvl ) ) return;

		// reduce the number of instantiations for log_impl by converting all string
		// types to string_views
		log_impl( lvl, detail::forward_as_string_view_if_possible( args )... );
	}

	template<class... ARGS>
	LIB_MART_COMMON_NO_INLINE void log_impl( Level lvl, ARGS&&... args )
	{
		_fillBuffer( lvl, AddNewline::Yes, std::forward<ARGS>( args )... );
		_writeBufferToSinks( lvl );
	}

	template<class... ARGS>
	inline void error_msg( ARGS&&... args )
	{
		log( Level::Error, std::forward<ARGS>( args )... );
	}

	template<class... ARGS>
	inline void status_msg( ARGS&&... args )
	{
		log( Level::Status, std::forward<ARGS>( args )... );
	}

	template<class... ARGS>
	inline void debug_msg( ARGS&&... args )
	{
		log( Level::Debug, std::forward<ARGS>( args )... );
	}

	template<class... ARGS>
	inline void trace_msg( ARGS&&... args )
	{
		log( Level::Trace, std::forward<ARGS>( args )... );
	}

	/* ### Change logging behavior #### */
	/**
	 * Gets the highest (TRACE > ERROR) log level with which messages are currently written to log
	 * @return current log level
	 */
	Level getLogLevel() const noexcept { return _currentLogLevel.load( std::memory_order_relaxed ); }
	void  setLogLevel( Level lvl ) noexcept { _currentLogLevel.store( lvl, std::memory_order_relaxed ); }

	/**
	 * Enables or disables logging, without changing log level - currently only way to prevent logging of errors
	 * @param enable
	 */
	void enable( bool enable = true ) noexcept { _enabled.store( enable, std::memory_order_relaxed ); }
	void disable() noexcept { enable( false ); }
	bool isEnabled() const noexcept { return _enabled; }

	void setName( const std::string_view name ) { _loggingName = _createLoggingName( name ); }

	/* ### Change sinks ###*/
	void addSink( std::shared_ptr<ILogSink> sink )
	{
		if( sink != nullptr ) { _sinks.emplace_back( std::move( sink ) ); }
	}
	void                                   clearSinks() { _sinks.clear(); }
	std::vector<std::shared_ptr<ILogSink>> getSinks() const { return _sinks; }

	/*### functions related to indendation level (mostly relevant for function call stack tracing) ###*/
	/**
	 * Increases indentation level for all following log messages created from this logger by two
	 */
	void bumpIndentLevel()
	{
		if( _spacer.size() < space_string_litteral.size() - 2 ) {
			_spacer = std::string_view( space_string_litteral.data(), _spacer.size() + 2 );
		}
	}

	/**
	 * Decreases indentation level for all following log messages created from this logger by two
	 */
	void removeIndentLevel()
	{
		if( _spacer.size() >= 2 ) { _spacer.remove_suffix( 2 ); }
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
			if( _log ) _log->removeIndentLevel();
		}
	};

	/**
	 * Same as bumpIdentLevel, but removes indentation level as soon as the returned guard object is destroyed
	 * This can be used to couple the log indentation level to a scope in an RAII manner
	 * @return Guard Object to whose lifetime the indentation level is coupled
	 */
	[[nodiscard]] IndentLevelGuard bumpIdentLevelGuarded() { return IndentLevelGuard( *this ); }

private:
	/*### Variables controlling logging behavior ###*/
	mart::copter_time_point     _startTime;
	mart::CopyableAtomic<Level> _currentLogLevel;
	mart::CopyableAtomic<bool>  _enabled;

	std::vector<std::shared_ptr<ILogSink>> _sinks;

	/*### Cached parts of logged message ### */
	mba::im_zstr     _loggingName; // This is what can be grepped for in the logfile
	std::string_view _spacer;

	static constexpr std::string_view space_string_litteral
		= "                                                                                                         ";

	static std::ostringstream& _sbuffer()
	{
		thread_local std::ostringstream stream;
		return stream;
	}

	// checks if a message  with priority <lvl> should be logged or not
	// Note, this is thread safe, but doesn't synchronize with e.g. disable()
	inline bool _shouldBeLogged( Level lvl )
	{
		// TODO: look at log Level of attached logger?
		return _enabled.load( std::memory_order_relaxed )
			   && ( lvl <= _currentLogLevel.load( std::memory_order_relaxed ) );
	}

	static mba::im_zstr _createLoggingName( const std::string_view moduleName, const std::string_view parentName = {} )
	{
		return mba::concat( parentName, "[", moduleName, "]" );
	}

	// Function where the actual message gets composed
	template<class... ARGS>
	void _fillBuffer( Level lvl, AddNewline newLine, ARGS&&... args )
	{
		std::ostream& buffer = _sbuffer();
		// line prefix
		formatForLog(
			buffer, lvl, " - At ", std::setw( 7 ), passedTime<milliseconds>( _startTime ), " - ", _loggingName, ": " );

		// Add thread Id and spacer in trace mode
		if( _currentLogLevel == Level::TRACE ) {
			ostream_flag_saver _( buffer );
			formatForLog( buffer, "[ThreadID: ", std::this_thread::get_id(), "]: ", _spacer );
		}

		// write actual message
		formatForLog( buffer, args... );

		// Append new line if requested
		if( newLine == AddNewline::Yes ) { buffer << '\n'; }
	}

	// write contents to all registered log sinks and reset buffer
	void _writeBufferToSinks( Level lvl )
	{
		auto text = _sbuffer().str();
		_sbuffer().str( std::string{} );
		for( const auto& se : _sinks ) {
			se->writeToLog( text, lvl );
		}
	}
};

// separate function, that can be forward declared
inline Logger& getDefaultLogger()
{
	return Logger::getDefaultLogger();
}

} /*namespace log*/
} /*namespace mart*/

#endif /* SRC_UTILS_MARTLOG_H_ */
