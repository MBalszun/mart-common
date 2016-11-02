#pragma once

#include <ostream>
#include <chrono>

namespace mart {
namespace _impl_print_chrono {

	//actual functions that take care of formatting
	inline void printChronoUnit(std::ostream& out, std::chrono::nanoseconds v)	{ out << v.count << "ns"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::microseconds v) { out << v.count << "us"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::milliseconds v) { out << v.count << "ms"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::seconds v)		{ out << v.count << "s"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::minutes v)		{ out << v.count << "min"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::hours v)		{ out << v.count << "h"; };

	//wrapper for duration for which  operator<< gets overloaded in such a way, that the correct suffix is appended
	template<typename rep, typename period>
	struct PrintableDuration {
		std::chrono::duration<rep, period> value;

		inline friend std::ostream& operator<<(std::ostream& out, const PrintableDuration& dur){
			printChronoUnit(out, dur.value);
		}
	};

}//_impl_print

/**
 * function that wrapps a std::chrono duration into a wrapper with overloaded  operator<< which allows printing of the variable
 * Use:
 *	std::cout << mart::sformat(std::chrono::seconds(100));
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


