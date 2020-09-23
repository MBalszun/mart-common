#ifndef LIB_MART_COMMON_GUARD_STRING_VIEW_H
#define LIB_MART_COMMON_GUARD_STRING_VIEW_H
/**
 * StringView.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: Provides mart::StringView, which is similar to std::string_view
 *
 */

/* ######## INCLUDES ######### */
#include <cassert>
#include <stdexcept>
#include <string>
#include <utility>
#include <string_view>

#ifdef __cpp_rtti
#include <typeinfo>
#endif

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

inline namespace sv_std {

struct StringView : std::string_view
{
	using std::string_view::string_view;
	constexpr StringView( std::string_view other ) noexcept
		: std::string_view( other )
	{
	}
	//std::string to_string() const { return std::string( *this ); }

	/**
	 * Splits the string at given position and returns a pair holding both substrings
	 *  - if 0 <= pos < size():
	 *		return substrings [0...pos) [pos ... size())
	 *	- if pos == size() or npos:
	 *		returns a copy of the current stringview and a default constructed one
	 *	- if pos > size()
	 *		throws std::out_of_range exception
	 */
	constexpr std::pair<StringView, StringView> split( size_t pos ) const noexcept
	{
		if( pos == npos || pos == size() ) { return std::pair<StringView, StringView>{*this, StringView{}}; }
		assert( pos < this->size() );
		return std::make_pair( StringView{this->data(), pos},
							   StringView{this->data() + pos + 1, this->size() - pos - 1} );
	}

	constexpr std::pair<StringView, StringView> split( char at ) const noexcept
	{
		std::string_view tmp{*this};
		auto             pos = tmp.find( at );
		if( pos == std::string_view::npos ) {
			return {*this, {}};
		} else {
			return {this->substr( 0, pos ), this->substr( pos + 1, this->size() - pos - 1 )};
		}
	}
};
} // namespace sv_std

// clang-format on
constexpr std::string_view EmptyStringView{""};
namespace _impl {
constexpr std::string_view ViewOfSpaces{
	"                                                                                                              "
	"                                                                                                              "
	"                                                                                                              "};
}
constexpr std::string_view getSpaces( size_t count )
{
	return _impl::ViewOfSpaces.substr( 0, count );
}

} // namespace mart

namespace std {
template<>
struct hash<mart::StringView> {
	// form http://stackoverflow.com/questions/24923289/is-there-a-standard-mechanism-to-retrieve-the-hash-of-a-c-string
	std::size_t operator()( mart::StringView str ) const noexcept
	{
		std::size_t h = 0;

		for( auto c : str ) {
			h += h * 65599 + c;
		}

		return h ^ ( h >> 16 );
	}
};
} // namespace std

namespace mart {

namespace details_to_integral {
// Core logic.
// Assuming number starts at first character and string is supposed to be parsed until first non-digit
template<class T>
constexpr T core( std::string_view str )
{
	T tmp = 0;
	for( auto c : str ) {
		int d = c - '0';
		if( d < 0 || 9 < d ) { break; }
		if( tmp >= std::numeric_limits<T>::max() / 16 ) { // quick check against simple constant
			if( tmp > ( std::numeric_limits<T>::max() - d ) / 10 ) {
#ifdef __cpp_rtti
				throw std::out_of_range( "String representing an integral (\"" + std::string( str )
										 + "\") overflows type " + typeid( T ).name() );

#else
				throw std::out_of_range( "String representing an integral (\"" + std::string( str )
										 + "\") overflows type in function" + __func__ );

#endif
			}
		}
		tmp = tmp * 10 + d;
	}
	return tmp;
}

// for unsigned types
template<class T>
constexpr auto base( const std::string_view str ) -> std::enable_if_t<std::is_unsigned<T>::value, T>
{
	assert( str.size() > 0 );
	if( str[0] == '+' ) {
		return details_to_integral::core<T>( str.substr( 1 ) );
	} else {
		return details_to_integral::core<T>( str );
	}
}

// for signed types
template<class T>
constexpr auto base( const std::string_view str ) -> std::enable_if_t<std::is_signed<T>::value, T>
{
	assert( str.size() > 0 );
	switch( str[0] ) {
		case '-': return -details_to_integral::core<T>( str.substr( 1 ) );
		case '+': return details_to_integral::core<T>( str.substr( 1 ) );
		default: return details_to_integral::core<T>( str );
	}
}
} // namespace details_to_integral

template<class T = int>
constexpr T to_integral( const std::string_view str )
{
	if( str.size() == 0 ) { return T{}; }
	return details_to_integral::base<T>( str );
}

// minimal implementation for sanitized strings that only contain digits (no negative numbers) and isn't too big for the
// return type
template<class T = unsigned int>
constexpr T to_integral_unsafe( std::string_view str )
{
	T value{};
	for( auto c : str ) {
		value = value * 10 + c - '0';
	}
	return value;
}

} // namespace mart

#endif // LIB_MART_COMMON_GUARD_STRING_VIEW_H
