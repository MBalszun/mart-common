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
#include <chrono>
#include <iomanip>
#include <ios>
#include <ostream>
#include <string_view>

/* Proprietary Library Includes */
/* Project Includes */
#include "./ArrayView.h"
#include "./StringView.h" //getSpaces
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

namespace _impl_print_chrono {

// actual functions that take care of formatting
// clang-format off
	inline void printChronoUnit(std::ostream& out, std::chrono::nanoseconds v)	{ out << v.count() << "ns"; }
	inline void printChronoUnit(std::ostream& out, std::chrono::microseconds v) { out << v.count() << "us"; }
	inline void printChronoUnit(std::ostream& out, std::chrono::milliseconds v) { out << v.count() << "ms"; }
	inline void printChronoUnit(std::ostream& out, std::chrono::seconds v)		{ out << v.count() << "s";  }
	inline void printChronoUnit(std::ostream& out, std::chrono::minutes v)		{ out << v.count() << "min";}
	inline void printChronoUnit(std::ostream& out, std::chrono::hours v)		{ out << v.count() << "h";  }
// clang-format on

// wrapper for duration for which  operator<< gets overloaded in such a way, that the correct suffix is appended
template<typename rep, typename period>
struct PrintableDuration {
	std::chrono::duration<rep, period> value;

	inline friend std::ostream& operator<<( std::ostream& out, const PrintableDuration& dur )
	{
		printChronoUnit( out, dur.value );
		return out;
	}
	template<class Dur>
	PrintableDuration<typename Dur::rep, typename Dur::period> as()
	{
		return { std::chrono::duration_cast<Dur>( value ) };
	}
};

} // namespace _impl_print_chrono

struct os_flag_guard {
	os_flag_guard( std::ostream& stream )
		: _stream( stream )
		, _flags( stream.flags() )
	{
	}
	~os_flag_guard() { _stream.flags( _flags ); }

	std::ostream&      _stream;
	std::ios::fmtflags _flags;
};

/**
 * function that wrapps a std::chrono duration into a wrapper with overloaded  operator<< which allows printing of the
 *variable Use: auto time = std::chrono::seconds(100); std::cout << mart::sformat(time);
 *
 * Output:
 *  100s
 */
template<typename rep, typename period>
inline auto sformat( std::chrono::duration<rep, period> dur ) -> _impl_print_chrono::PrintableDuration<rep, period>
{
	return _impl_print_chrono::PrintableDuration<rep, period>{ dur };
}

template<typename rep, typename period, typename repto = rep, typename periodto = period>
inline auto sformat( std::chrono::duration<rep, period> dur, std::chrono::duration<repto, periodto> )
	-> _impl_print_chrono::PrintableDuration<repto, periodto>
{
	return _impl_print_chrono::PrintableDuration<repto, periodto>{
		std::chrono::duration_cast<std::chrono::duration<repto, periodto>>( dur ) };
}

enum class Pad : std::uint8_t { Middle, Left, Right };

namespace _impl_print {
struct PaddedStringView {

	const std::string_view string;
	const std::size_t      total_length;
	const Pad              side;

	inline friend std::ostream& operator<<( std::ostream& out, const PaddedStringView& e )
	{
		switch( e.side ) {
			case Pad::Left: {
				const size_t space_cnt = e.total_length <= e.string.size() ? 0 : e.total_length - e.string.size();
				out << mart::getSpaces( space_cnt );
				out << e.string;
				break;
			}
			case Pad::Right: {
				const size_t space_cnt = e.total_length <= e.string.size() ? 0 : e.total_length - e.string.size();
				out << e.string;
				out << mart::getSpaces( space_cnt );
				break;
			}
			case Pad::Middle: {
				const size_t space_cnt = e.total_length <= e.string.size() ? 0 : e.total_length - e.string.size();
				const size_t left_cnt  = space_cnt / 2;
				const size_t right_cnt = space_cnt - left_cnt;
				out << mart::getSpaces( left_cnt );
				out << e.string;
				out << mart::getSpaces( right_cnt );
			}
		}
		return out;
	}
};

struct data_fmt_info {
	std::size_t chunck_size     = 8;
	std::size_t row_size        = 32;
	char        start_delimiter = '[';
	char        end_delimiter   = ']';
};
static constexpr data_fmt_info default_data_fmt{};

struct formatted_data_range {
	mart::ConstMemoryView range;
	data_fmt_info         fmt_info;
};

inline std::ostream& operator<<( std::ostream& out, const formatted_data_range& range )
{
	os_flag_guard g( out );

	out << range.fmt_info.start_delimiter << ' ';

	for( std::size_t i = 0; i < range.range.size(); ++i ) {
		out << std::hex << std::setw( 2 ) << std::setfill( '0' ) << (int)range.range[i] << ' ';
		if( ( i + 1 ) % range.fmt_info.row_size == 0 ) {
			out << "\n  ";
		} else {
			if( ( i + 1 ) % range.fmt_info.chunck_size == 0 ) { out << "  "; }
		}
	}
	out << range.fmt_info.end_delimiter;

	return out;
}
} // namespace _impl_print

inline auto padded( std::string_view str, size_t total_length, Pad pad ) -> _impl_print::PaddedStringView
{
	return { str, total_length, pad };
}

inline auto sformat( mart::ConstMemoryView range, _impl_print::data_fmt_info fmt = _impl_print::default_data_fmt )
{
	return _impl_print::formatted_data_range{ range, fmt };
}

namespace chrono_ostream_op_overloads {

// clang-format off
inline std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds v)	{ out << v.count() << "ns";  return out; }
inline std::ostream& operator<<(std::ostream& out, std::chrono::microseconds v) { out << v.count() << "us";  return out; }
inline std::ostream& operator<<(std::ostream& out, std::chrono::milliseconds v) { out << v.count() << "ms";  return out; }
inline std::ostream& operator<<(std::ostream& out, std::chrono::seconds v)		{ out << v.count() << "s";   return out; }
inline std::ostream& operator<<(std::ostream& out, std::chrono::minutes v)		{ out << v.count() << "min"; return out; }
inline std::ostream& operator<<(std::ostream& out, std::chrono::hours v)		{ out << v.count() << "h";   return out; }
// clang-format on

} // namespace chrono_stream_op_overloads

} // namespace mart

#endif
