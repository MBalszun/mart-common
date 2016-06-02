#pragma once
/**
 * MartTime.h
 *
 *  @date: 2016.04.22
 *  @author: Michael Balszun <michael.balszun@mytum.de>
 *
 *  @brief Time related software for software on the mart payload computer
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
#include <type_traits>

/* Proprietary Library Includes */

/* Project Includes */

namespace mart {

	/**
	 * Default clock that is used by all other functions and classes
	 */
	using copter_clock = std::chrono::system_clock;
	using copter_time_point = copter_clock::time_point;
	using copter_default_period = copter_clock::duration;

	//small wrapper to ensure consistent use of times
	inline copter_time_point now() noexcept {
		return copter_clock::now();
	}

	/**
	 * For situations, where you want to express a point in time
	 * in the classic posix way: as duration since epoch
	 * @return
	 */
	template<class T = copter_default_period>
	T timeSinceEpoch(){ return std::chrono::duration_cast<T>	(mart::now().time_since_epoch()); }

	/*### for interfacing with legacy code, that expects integers representing a duration since epoch ###*/
	inline int64_t us_SinceEpoch()	{ return timeSinceEpoch<std::chrono::microseconds>().count();	}
	inline int64_t ms_SinceEpoch()	{ return timeSinceEpoch<std::chrono::milliseconds>().count();	}
	inline int64_t s_SinceEpoch()	{ return timeSinceEpoch<std::chrono::seconds>().count();		}

	/**
	 * Time that has elapsed since time_point in user units that can be defined by the user
	 * @param start: point in time
	 * @return
	 */
	template<class TIME_T = copter_default_period>
	TIME_T passedTime(copter_time_point start){
		return std::chrono::duration_cast<TIME_T>(mart::now() - start);
	}

	/**
	 * Function that gives a readable name to common check (if mart::Timer is not used)
	 * @param start
	 * @param timeout
	 * @return
	 */
	inline bool hasTimedOut(copter_time_point start, copter_default_period timeout){
		return (mart::now()-start) > timeout;
	}

	/**
	 * Class to keep track of elapsed time / timeouts
	 */
	struct Timer {
		Timer() :
			_start_time(mart::now())
		{}

		Timer(copter_default_period timeout) :
			_start_time(mart::now()),
			_timeout{timeout}
		{}

		/// Resets the timer and returns the elapsed time as if calling elapsed right before the reset
		copter_default_period reset() {
			auto t = elapsed();
			_start_time = mart::now();
			return t;
		}

		/// time elapsed since creation of Timer or last call to reset
		template<class DUR = copter_default_period>
		DUR elapsed() const {
			return std::chrono::duration_cast<DUR>(now()-_start_time);
		}

		/// remaining time, before hasTimedOut will be true (will always return a non-negative number)
		template<class DUR = copter_default_period>
		DUR remaining() const {
			return std::max(std::chrono::duration_cast<DUR>(_timeout-(now()-_start_time)),DUR{});
		}

		/// true if duration since creation or last call to reset is longer than the timeout that was specified upon creation
		bool hasTimedOut() const {
			using namespace std::chrono;
			return (mart::now()-_start_time) > _timeout;
		}
	private:
		copter_time_point _start_time;
		copter_default_period _timeout{-1};
	};




};



#endif /* SRC_UTILS_MARTTIME_H_ */
