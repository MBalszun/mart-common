#ifndef LIB_MART_COMMON_GUARD_PRINT_WRAPPERS_H
#define LIB_MART_COMMON_GUARD_PRINT_WRAPPERS_H
/**
 * PrintWrappers.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: Wrapper objects that add formatting information to a type
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <ostream>
#include <chrono>

/* Proprietary Library Includes */
/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace _impl_print_chrono {

	//actual functions that take care of formatting
	inline void printChronoUnit(std::ostream& out, std::chrono::nanoseconds v)	{ out << v.count() << "ns"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::microseconds v) { out << v.count() << "us"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::milliseconds v) { out << v.count() << "ms"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::seconds v)		{ out << v.count() << "s"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::minutes v)		{ out << v.count() << "min"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::hours v)		{ out << v.count() << "h"; };

	//wrapper for duration for which  operator<< gets overloaded in such a way, that the correct suffix is appended
	template<typename rep, typename period>
	struct PrintableDuration {
		std::chrono::duration<rep, period> value;

		inline friend std::ostream& operator<<(std::ostream& out, const PrintableDuration& dur){
			printChronoUnit(out, dur.value);
			return out;
		}
	};

}//_impl_print

/**
 * function that wrapps a std::chrono duration into a wrapper with overloaded  operator<< which allows printing of the variable
 * Use:
 *  auto time = std::chrono::seconds(100);
 *	std::cout << mart::sformat(time);
 *
 * Output:
 *  100s
 */
template<typename rep, typename period>
inline auto sformat(std::chrono::duration<rep,period> dur) -> _impl_print_chrono::PrintableDuration<rep, period>
{
	return  _impl_print_chrono::PrintableDuration<rep, period>{ dur };
}

}//mart

#endif


