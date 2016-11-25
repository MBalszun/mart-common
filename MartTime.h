#pragma once
/**
 * MartTime.h
 *
 *  @date: 2016.04.22
 *  @author: Michael Balszun <michael.balszun@mytum.de>
 *
 *  @brief Time related functions
 *
 *  This file mainly provides some simple wrappers around common,
 *  but rather verbose expressions related to the std::chrono
 *  facilities (in particular measuring time).
 *
 *  e.g. if you want to have a timeout for a certain loop:
 *
 *  void foo(std::chrono::milliseconds timeout) {
 *  	mart::Timer timer(timeout);
 *
 *  	while (!timer.hasTimedOut()) {
 *  		//Do something
 *  	}
 *  }
 *
 *  equivalent without timer:
 *
 *  void foo(std::chrono::milliseconds timeout) {
 *  	auto start = std::chrono::system_clock::now();
 *
 *  	while ((std::chrono::system_clock::now() - start) < timeout) {
 *  		//Do something
 *  	}
 *  }
 *
 *  It should also increase consistency by defining a default clock and default duration
 *
 *  Please report any bugs or feature requests to michael.balszun@mytum.de
 *
 *  IMPELEMENTATION_NOTE: std::chrono::system_clock was chosen as  the default clock for it's compatibility
 *  with c APIs (like to_time_t)
 *
 *  TODO:
 *  	- Timestamp class, whose representation and point of reference (epoch) is standardized across all systems
 *  	  (so it can be used as part of packets send over the network)
 *  	- LoopTimer: Class that gives easy access to execution time of loop and statistics over a window like wcet, average, frametime ...
 *
 *
 */

#ifndef SRC_UTILS_MARTTIME_H_
#define SRC_UTILS_MARTTIME_H_

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <chrono>
#include <thread>
#include <type_traits>
#include <algorithm>
/* Proprietary Library Includes */

/* Project Includes */

namespace mart {
// some shorthands
using std::chrono::hours;
using std::chrono::minutes;
using std::chrono::seconds;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

//replacement for the not-yet supported c++14's chrono literals
namespace chrono_literals {
// clang-format off
constexpr mart::hours			operator "" _h	(unsigned long long v) { return mart::hours(v); }
constexpr mart::minutes			operator "" _min(unsigned long long v) { return mart::minutes(v); }
constexpr mart::seconds			operator "" _s	(unsigned long long v) { return mart::seconds(v); }
constexpr mart::milliseconds	operator "" _ms	(unsigned long long v) { return mart::milliseconds(v); }
constexpr mart::microseconds	operator "" _us	(unsigned long long v) { return mart::microseconds(v); }
constexpr mart::nanoseconds		operator "" _ns	(unsigned long long v) { return mart::nanoseconds(v); }
// clang-format on
};


/**
 * Default clock that is used by all other functions and classes
 */
using copter_clock			= std::chrono::system_clock;
using copter_time_point		= copter_clock::time_point;
using copter_default_period = copter_clock::duration;

//small wrapper to ensure consistent use of times
inline copter_time_point now() noexcept
{
	return copter_clock::now();
}

/**
 * For situations, where you want to express a point in time
 * in the classic posix way: as duration since epoch
 */
template <class T = copter_default_period>
T timeSinceEpoch()
{
	return std::chrono::duration_cast<T>( mart::now().time_since_epoch() );
}

/*### for interfacing with legacy code, that expects integers representing a duration since epoch ###*/

// clang-format off
inline int64_t us_SinceEpoch() { return timeSinceEpoch<std::chrono::microseconds>().count(); }
inline int64_t ms_SinceEpoch() { return timeSinceEpoch<std::chrono::milliseconds>().count(); }
inline int64_t s_SinceEpoch()  { return timeSinceEpoch<std::chrono::seconds>().count(); }
// clang-format on

/**
 * Time that has elapsed since \p start. Units that can be defined by the user
 * @param start: point in time
 *
 * Example:
 * auto t1 = mart::now();
 *
 * //do something
 *
 * std::cout << passedTime<std::chrono::milliseconds>(t1).count << "milliseconds have passed since begin of function\n");
 *
 */
template <class TIME_T = copter_default_period>
TIME_T passedTime( copter_time_point start )
{
	return std::chrono::duration_cast<TIME_T>( mart::now() - start );
}

/**
 * Function that gives a readable name to common check (if mart::Timer is not used anyway)
 * @param start
 * @param timeout
 * @return result of (mart::now() - \p start) > timeout
 */
inline bool hasTimedOut( copter_time_point start, copter_default_period timeout )
{
	return ( mart::now() - start ) > timeout;
}

/**
 * Class to keep track of elapsed time / timeouts
 */
struct Timer {
	Timer()
		: _start_time( now() )
	{
	}

	explicit Timer( copter_default_period timeout )
		: _start_time(now() )
		, _timeout{timeout}
	{
	}

	/// Resets the timer and returns the elapsed time as if calling elapsed right before the reset
	copter_default_period reset()
	{
		auto t		= elapsed();
		_start_time = mart::now();
		return t;
	}

	/// time elapsed since creation of Timer or last call to reset
	template <class DUR = copter_default_period>
	DUR elapsed() const
	{
		return std::chrono::duration_cast<DUR>( now() - _start_time );
	}

	/// remaining time, before hasTimedOut will be true (will always return a non-negative number)
	template <class DUR = copter_default_period>
	DUR remaining() const
	{
		return std::max( std::chrono::duration_cast<DUR>( _timeout - ( now() - _start_time ) ), DUR{} );
	}

	/// true if duration since creation or last call to reset is longer than the timeout that was specified upon creation
	bool hasTimedOut() const
	{
		using namespace std::chrono;
		return ( mart::now() - _start_time ) > _timeout;
	}

private:
	copter_time_point	 _start_time;
	copter_default_period _timeout{-1};
};

/**
 * Can e.g. be used to periodically execute a loop body:
 *
 * for (mart::PeriodicScheduler sched(std::chrono::seconds(2)) ; sched.invocationCnt() < 100 ; sched.sleep()) {
 *   // loop body
 * }
 *
 * if loop body takes longer to execute than the period the thread will not sleep, until start_time + sched.invocationCnt()*period > mart::now()
 *
 */
class PeriodicScheduler {
public:
	explicit PeriodicScheduler( microseconds interval )
		: _interval( interval )
	{
		_lastInvocation = mart::now();
	}

	copter_time_point getNextWakeTime() const
	{
		return _lastInvocation + _interval;
	}

	void sleep()
	{
		_lastInvocation += _interval;
		std::this_thread::sleep_until( _lastInvocation );
		_cnt++;
	}

	size_t invocationCnt() const
	{
		return _cnt;
	}

	/// time elapsed since creation of Timer or last call to reset
	template <class DUR = copter_default_period>
	DUR runtime() const
	{
		return std::chrono::duration_cast<DUR>(now() - _lastInvocation + _interval*(_cnt - 1));
	}

private:
	microseconds		_interval;
	copter_time_point		  _lastInvocation;
	size_t					  _cnt = 1;
};

} //mart

#endif /* SRC_UTILS_MARTTIME_H_ */
