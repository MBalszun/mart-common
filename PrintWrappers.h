#pragma once

#include <ostream>
#include <chrono>

namespace mart {
namespace _impl_print_chrono {

	inline void printChronoUnit(std::ostream& out, std::chrono::nanoseconds v)	{ out << v.count << "ns"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::microseconds v) { out << v.count << "us"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::milliseconds v) { out << v.count << "ms"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::seconds v)		{ out << v.count << "s"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::minutes v)		{ out << v.count << "min"; };
	inline void printChronoUnit(std::ostream& out, std::chrono::hours v)		{ out << v.count << "h"; };


	template<typename rep, typename period>
	struct PrintableDuration {
		std::chrono::duration<rep, period> value;

		friend std::ostream& operator<<(std::ostream& out, const PrintableDuration& dur){
			printChronoUnit(out, dur.value);
		}
	};

}//_impl_print

/**
 * Use:
 *	std::cout << mart::sformat(std::chrono::seconds(100)) << 'n';
 *
 * Output:
 *  100s
 */
template<typename rep, typename period>
_impl_print_chrono::PrintableDuration<rep,period> sformat(std::chrono::duration<rep,period> dur)
{
	return{ dur };
}

}//mart


