#ifndef LIB_MART_COMMON_GUARD_LOGGING_DEFAULT_FORMATTER_H
#define LIB_MART_COMMON_GUARD_LOGGING_DEFAULT_FORMATTER_H
/**
 * default_formatter.h (mart-common/logging)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provide default formatting functions when datatypes are logged - those can be overridden by the user
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <chrono>
#include <ostream>

/* Proprietary Library Includes */
#include "../ArrayView.h"

/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */



namespace mart {
namespace log {

class ostream_flag_saver final {
public:
	explicit ostream_flag_saver( std::ostream& stream )
		: _stream( &stream )
		, _flags( stream.flags() )
		, _fillc( stream.fill() )
	{
	}
	~ostream_flag_saver()
	{
		_stream->fill( _fillc );
		_stream->flags( _flags );
	}

private:
	std::ostream*	   _stream;
	std::ios::fmtflags _flags;
	char			   _fillc;
};

/**
 * function template that is used for writing a parameter to output buffer.
 *
 * Defaults to operator<<(ostream,value) but can be overloaded for own data type
 */

// clang-format off
template <class T>
inline void defaultFormatForLog(std::ostream& out, const T& value)					{ out << value; }

//overload for char types, such that e.g. uint8_t variables are printed as numbers and not the characters
inline void defaultFormatForLog(std::ostream& out, char value)						{ out << (int)value; }
inline void defaultFormatForLog(std::ostream& out, signed char value)				{ out << (int)value; }
inline void defaultFormatForLog(std::ostream& out, unsigned char value)				{ out << (int)value; }

//overload for chrono types
inline void defaultFormatForLog(std::ostream& out, std::chrono::nanoseconds value)	{ out << value.count() << "ns"; }
inline void defaultFormatForLog(std::ostream& out, std::chrono::microseconds value) { out << value.count() << "us"; }
inline void defaultFormatForLog(std::ostream& out, std::chrono::milliseconds value) { out << value.count() << "ms"; }
inline void defaultFormatForLog(std::ostream& out, std::chrono::seconds value)		{ out << value.count() << "s"; }
inline void defaultFormatForLog(std::ostream& out, std::chrono::minutes value)		{ out << value.count() << "min"; }
inline void defaultFormatForLog(std::ostream& out, std::chrono::hours value)		{ out << value.count() << "h"; }

template<class Clock, class Dur>
inline void defaultFormatForLog(std::ostream& out, std::chrono::time_point<Clock, Dur> value) { defaultFormatForLog(out, value.time_since_epoch()); }

//make sure that string literals are not taken by the ConstMemoryView overload
template <size_t N>
inline void defaultFormatForLog(std::ostream& out, const char(&stringLit)[N])		{ out << mart::StringView(stringLit); }

// clang-format on

inline void defaultFormatForLog( std::ostream& out, std::thread::id id )
{
	ostream_flag_saver _( out );
	out << std::hex << "0x" << id;
}

//Default for ArrayViews [a, b, c, d, ]
template <class T>
inline void defaultFormatForLog( std::ostream& out, mart::ArrayView<T> arr )
{
	out << '[';
	bool first = true;
	for ( auto&& e : arr ) {
		if ( first ) {
			first = false;
		} else {
			out << ", ";
		}
		formatForLog( out, e );
	}
	out << ']';
}

namespace _impl_log {
inline void printOneLine( std::ostream& out, mart::ConstMemoryView mem, size_t fillto = 0 )
{
	//you can use space.substr(0,x)
	constexpr StringView space("                                                                                                                                                     ");

	out << '[';
	for ( uint8_t b : mem ) {
		out << ' ' << std::setw( 2 ) << (int)b;
	}
	if ( fillto > mem.size() ) {
		out << space.substr( 0, ( fillto - mem.size() ) * 3 );
	}
	out << " ]";
}
}

inline void defaultFormatForLog( std::ostream& out, mart::ConstMemoryView mem )
{
	ostream_flag_saver _( out );
	out << std::right << std::hex << std::setfill( '0' );
	bool oneLine = mem.size() <= 20;
	if ( oneLine ) {
		_impl_log::printOneLine( out, mem );
	} else {
		while ( !mem.empty() ) {
			out << "\n\t";
			auto t = mem.subview( 0, std::min<size_t>( 20ul, mem.size() ) );
			_impl_log::printOneLine( out, t, 20 );
			mem = mem.subview( t.size() );
		}
		out << '\n';
	}
}

/**
 * function template that is used for writing a parameter to output buffer.
 *
 * Defaults to defaultFormatForLog, which defaults to operator<<(ostream,value) but can be overloaded for own data type
 *
 * @param out
 * @param value
 */
template <class T>
inline void formatForLog( std::ostream& out, const T& value )
{
	//fallback to default formatters
	defaultFormatForLog( out, value );
}

// overloads for types in the mart::log namespace
inline void formatForLog( std::ostream& out, const LOG_LVL& value )
{
	out << std::right << std::setw( 6 ) << toString( value );
}
inline void formatForLog( std::ostream& out, std::thread::id id )
{
	ostream_flag_saver _( out );
	out << std::hex << "0x" << id;
}

template <class... ARGS>
inline void formatForLog( std::ostream& out, const ARGS&... args )
{
	/* Hack to put all arguments into stream, without recursion
	 * Creates an integer array of zeros
	 * and as a "by-product" adds each argument to the stream (which is actually what we want)
	 */
	int dummy[]{( formatForLog( out, args ), 0 )...};
	(void)dummy; //silence warning about unused array
}
}
}

#endif