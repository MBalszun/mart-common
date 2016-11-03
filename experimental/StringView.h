/*
 * StringView.h
 *
 *  Created on: Jun 20, 2016
 *      Author: balszun
 */

#ifndef LIBS_MART_COMMON_EXPERIMENTAL_STRINGVIEW_H_
#define LIBS_MART_COMMON_EXPERIMENTAL_STRINGVIEW_H_
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <cstring>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>

/* Proprietary Library Includes */

/* Project Includes */
#include "ArrayViewAdaptor.h"

namespace mart {

class StringView : public ArrayViewAdaptor<const char, StringView> {
public:
	// type defs
	using CharT		  = char;
	using traits_type = std::char_traits<CharT>;

	static constexpr size_type npos = size_type( -1 );

public:
	/* #### CTORS #### */
	constexpr StringView() = default;

	StringView( const std::string& other ) noexcept
		: _start( other.data() )
		, _size( other.size() )
	{
	}

	constexpr StringView( const char* other, size_type size ) noexcept
		: _start( other )
		, _size( size )
	{
	}

	static StringView fromZString( const char* other ) { return {other, std::strlen( other )}; }

	// NOTE: Use only for string literals!!!
	template <size_t N>
	constexpr StringView( const char ( &other )[N] ) noexcept
		: _start( other )
		, _size( N - 1 )
	{
	}

	template <class T>
	StringView( const T* const& other ) = delete;

	/* #### Special member functions #### */
	constexpr StringView( const StringView& other ) = default;
	StringView& operator=( const StringView& other ) = default;

	/*#### string functions ####*/
	std::string to_string() const { return std::string( cbegin(), cend() ); }

	constexpr StringView substr( size_t offset, size_t count ) const
	{
		return count == npos
				   ? substr( offset )
				   : offset + count <= this->_size
						 ? StringView{this->_start + offset, count}
						 : throw std::out_of_range(
							   "Tried to create a substring that would exceed the original string. Original string:\n\n"
							   + this->to_string() + "\"\n" );
	}

	constexpr StringView substr( size_t offset ) const { return substr( offset, size() - offset ); }

	/*#### algorithms ####*/

	size_type find( StringView str, size_type pos = 0 ) const
	{
		if( pos + str.size() >= size() ) return npos;
		auto it = std::search( this->cbegin() + pos, this->cend(), str.cbegin(), str.cend() );
		return it != this->cend() ? it - this->cbegin() : npos;
	}
	size_type find( char c, size_type pos = 0 ) const
	{
		if( pos + 1 >= size() ) return npos;
		auto it = std::find( this->cbegin() + pos, this->cend(), c );
		return it != this->cend() ? it - this->cbegin() : npos;
	}

	friend int compare( StringView l, StringView r );
	friend std::ostream& operator<<( std::ostream& out, const StringView string )
	{
		out.write( string.data(), string.size() );
		return out;
	}

protected:
	friend class ArrayViewAdaptor<const char, StringView>;
	constexpr size_type		_arrayView_size() const { return _size; }
	constexpr const_pointer _arrayView_data() const { return _start; }

	const char* _start = nullptr;
	size_type   _size  = 0;
};

inline int compare( StringView l, StringView r )
{
	if( ( l._start == r._start ) && ( l.size() == l.size() ) ) {
		return 0;
	}
	int ret = StringView::traits_type::compare( l.cbegin(), r.cbegin(), std::min( l.size(), r.size() ) );

	if( ret == 0 ) {
		// couldn't find a difference yet -> compare sizes
		if( l.size() < r.size() ) {
			ret = -1;
		} else if( l.size() > r.size() ) {
			ret = 1;
		}
	}
	return ret;
}

/* operator overloads */
// clang-format off
inline bool operator==(const StringView& l, const StringView& r) { return compare(l,r) == 0; }
inline bool operator!=(const StringView& l, const StringView& r) { return !(l == r); }
inline bool operator< (const StringView& l, const StringView& r) { return compare(l,r) < 0; }
inline bool operator> (const StringView& l, const StringView& r) { return r<l; }
inline bool operator<=(const StringView& l, const StringView& r) { return !(l>r); }
inline bool operator>=(const StringView& l, const StringView& r) { return !(l < r); }
// clang-format on
constexpr StringView EmptyStringView{""};
}

namespace std {
template <>
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
}

#endif /* LIBS_MART_COMMON_EXPERIMENTAL_STRINGVIEW_H_ */
