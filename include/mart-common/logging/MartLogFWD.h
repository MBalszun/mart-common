#ifndef LIB_MART_COMMON_GUARD_LOGGING_MART_LOG_FWD_H
#define LIB_MART_COMMON_GUARD_LOGGING_MART_LOG_FWD_H

#define MART_LOG_LOG_LVL_ERROR 0
#define MART_LOG_LOG_LVL_STATUS 1
#define MART_LOG_LOG_LVL_DEBUG 2
#define MART_LOG_LOG_LVL_TRACE 3

#ifndef MART_LOG_MAX_LOG_LVL
	//Every MARTE_LOG_<LVL> macro with a higher level (numerical number) will be compiled to a nop
	#define MART_LOG_MAX_LOG_LVL MART_LOG_LOG_LVL_DEBUG
	//#define MART_LOG_MAX_LOG_LVL MART_LOG_LOG_LVL_TRACE
#endif

/**
 * Workaround for error in Visual studio's variadic argument expansion policy.
 *
 * Usage:
 *
 *	#define MY_MACRO(X,Y,Z) X*Y*Z
 *  #define MY_V_MACRO (...) MY_MACRO(__VA_ARGS__)
 *
 *  becomes
 *
 *	#define MY_MACRO(X,Y,Z) X*Y*Z
 *  #define MY_V_MACRO (...) MART_LOG_IMPL_EXPAND(MY_MACRO(__VA_ARGS__))
 *
 **/
#define MART_LOG_IMPL_EXPAND( X ) X

#define MART_DEFLOG (::mart::log::Logger::getDefaultLogger())

#define MART_LOG_ERROR(LOGGER,...)  		((LOGGER).log(mart::log::LOG_LVL::ERROR,__VA_ARGS__))
#define MART_DEFLOG_ERROR(...)  		 (MART_DEFLOG.log(mart::log::LOG_LVL::ERROR,__VA_ARGS__))
#define MART_LOG_ERROR_COND(COND,...)		do{ if (COND) { MART_LOG_IMPL_EXPAND(MART_LOG_ERROR(__VA_ARGS__));} } while(false)
#define MART_DEFLOG_ERROR_COND(COND,...)	do{ if (COND) { MART_LOG_IMPL_EXPAND(MART_DEFLOG_ERROR(__VA_ARGS__));} } while(false)

#if MART_LOG_MAX_LOG_LVL >= MART_LOG_LOG_LVL_STATUS
	#define MART_LOG_STATUS(LOGGER,...) 	((LOGGER).log(mart::log::LOG_LVL::STATUS,__VA_ARGS__))
	#define MART_DEFLOG_STATUS(...) 	 (MART_DEFLOG.log(mart::log::LOG_LVL::STATUS,__VA_ARGS__))
	#define MART_LOG(...)				 (MART_DEFLOG.log(mart::log::LOG_LVL::STATUS,__VA_ARGS__))
	#define MART_LOG_STATUS_COND(COND,...)		do{ if (COND) { MART_LOG_IMPL_EXPAND(MART_LOG_STATUS(__VA_ARGS__)); } } while(false)
	#define MART_DEFLOG_STATUS_COND(COND,...)	do{ if (COND) { MART_LOG_IMPL_EXPAND(MART_DEFLOG_STATUS(__VA_ARGS__)); } } while(false)
#else
	#define MART_LOG_STATUS(LOGGER,...)			(void)0
	#define MART_DEFLOG_STATUS(...) 			(void)0
	#define MART_LOG(...)						(void)0
	#define MART_LOG_STATUS_COND(COND,...)		(void)0
	#define MART_DEFLOG_STATUS_COND(COND,...)	(void)0
#endif

#if MART_LOG_MAX_LOG_LVL >= MART_LOG_LOG_LVL_DEBUG
	#define MART_LOG_DEBUG(LOGGER,...)  	((LOGGER).log(mart::log::LOG_LVL::DEBUG,__VA_ARGS__))
	#define MART_DEFLOG_DEBUG(...)  	 (MART_DEFLOG.log(mart::log::LOG_LVL::DEBUG,__VA_ARGS__))
	#define MART_LOG_DEBUG_COND(COND,...)		do{ if (COND) {  MART_LOG_IMPL_EXPAND(MART_LOG_DEBUG(__VA_ARGS__));} } while(false)
	#define MART_DEFLOG_DEBUG_COND(COND,...)	do{ if (COND) {  MART_LOG_IMPL_EXPAND(MART_DEFLOG_DEBUG(__VA_ARGS__));} } while(false)
#else
	#define MART_LOG_DEBUG(LOGGER,...) 			(void)0
	#define MART_DEFLOG_DEBUG(...) 				(void)0
	#define MART_LOG_DEBUG_COND(COND,...)		(void)0
	#define MART_DEFLOG_DEBUG_COND(COND,...)	(void)0
#endif

#if MART_LOG_MAX_LOG_LVL >= MART_LOG_LOG_LVL_TRACE
	#define MART_LOG_TRACE(LOGGER,...)  		((LOGGER).log(mart::log::LOG_LVL::TRACE,__VA_ARGS__))
	#define MART_DEFLOG_TRACE(...)  		 (MART_DEFLOG.log(mart::log::LOG_LVL::TRACE,__VA_ARGS__))
	#define MART_LOG_TRACE_COND(COND,...)		do{ if (COND) {  MART_LOG_IMPL_EXPAND(MART_LOG_TRACE(__VA_ARGS__)); } } while(false)
	#define MART_DEFLOG_TRACE_COND(COND,...)	do{ if (COND) {  MART_LOG_IMPL_EXPAND(MART_DEFLOG_TRACE(__VA_ARGS__)); } }while(false)
#else
	#define MART_LOG_TRACE(LOGGER,...) 			(void)0
	#define MART_DEFLOG_TRACE(...) 				(void)0
	#define MART_LOG_TRACE_COND(COND,...)		(void)0
	#define MART_DEFLOG_TRACE_COND(COND,...)	(void)0
#endif


namespace mart
{
	namespace log
	{
		class Logger;

		Logger& getDefaultLogger();
	}
}

#endif
