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
/* Standard Library Includes */
#include <algorithm>
#include <cstring>
#include <cassert>
#include <memory>
#include <iosfwd>
#include <string>
#include <string_view>

/* Proprietary Library Includes */
#include "./cpp_std/type_traits.h"

/* Project Includes */
#include "ArrayViewAdaptor.h"
#include "algorithm.h"

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

class StringView : public ArrayViewAdaptor<const char, StringView> {
public:
	// type defs
	using CharT = char;
	using traits_type = std::char_traits<CharT>;

	static constexpr size_type npos = size_type(-1);

	/* #### CTORS #### */
	constexpr StringView() noexcept = default;

	constexpr operator std::string_view() const noexcept {
		return std::string_view(_start, _size);
	}

	StringView(const std::string& other) noexcept
		: _start(other.data())
		, _size(other.size())
	{
	}

	constexpr explicit StringView( const std::string_view other ) noexcept
		: _start( other.data() )
		, _size( other.size() )
	{
	}

	constexpr StringView(const char* other, size_type size) noexcept
		: _start(other)
		, _size(size)
	{
	}

	constexpr static StringView fromZString(const char* other) { return{ other, std::string_view(other).size() }; }

	// NOTE: Use only for string literals!!!
	template <size_t N>
	constexpr StringView(const char(&other)[N]) noexcept
		: _start(other)
		, _size(N - 1)
	{
	}

	constexpr StringView(const char &other) noexcept
		: _start(&other)
		, _size(1)
	{
	}

	//prevent construction from integral type except char
	template<class T, class = typename std::enable_if<std::is_integral<T>::value && (!std::is_same<T, char>::value)>::type >
	constexpr StringView(const T &other) = delete;

	template <class T>
	StringView(const T* const& other) = delete;

	/* #### Special member functions #### */
	constexpr StringView(const StringView& other) noexcept = default;
	constexpr StringView& operator=(const StringView& other) noexcept = default;
	constexpr StringView(StringView&& other) noexcept = default;
	constexpr StringView& operator=( StringView&& other ) noexcept = default;

	/*#### string functions ####*/
	std::string to_string() const { return std::string(this->cbegin(), this->cend()); }

	constexpr StringView substr( size_t offset, size_t count = npos ) const noexcept
	{
		assert( offset <= _size );
		if( count == npos ) {
			return StringView {this->_start + offset, this->_size - offset};
		}
		assert( offset + count <= _size );
		return StringView {this->_start + offset,count};
	}

	/**
	* Splits the string at given position and returns a pair holding both substrings
	*  - if 0 <= pos < size():
	*		return substrings [0...pos) [pos ... size())
	*	- if pos == size() or npos:
	*		returns a copy of the current stringview and a default constructed one
	*	- if pos > size()
	*		throws std::out_of_range exception
	*/
	constexpr std::pair<StringView, StringView> split(size_t pos) const noexcept
	{
		if (pos == npos || pos == _size) { return std::pair<StringView, StringView> {*this, StringView {}};
		}
		assert( pos < _size );
		return std::make_pair( StringView {this->_start, pos}, StringView {this->_start + pos + 1, _size - pos - 1} );
	}

	constexpr std::pair<StringView, StringView> split(char at) const noexcept
	{
		std::string_view tmp {*this};
		auto             pos = tmp.find( at );
		if( pos == std::string_view::npos ) {
			return {*this, {}};
		} else {
			return {this->substr( 0, pos ), this->substr( pos + 1, this->size() - pos - 1 )};
		}
	}

	/*#### algorithms ####*/

	size_type find(char c, size_type start_pos = 0) const
	{
		if (start_pos >= size()) {
			return npos;
		}

		const size_t pos = std::find(this->cbegin() + start_pos, this->cend(), c) - this->cbegin();
		return pos < this->size() ? pos : npos;
	}

	template <class P>
	size_type find_if(P p, size_type start_pos = 0) const
	{
		if (start_pos >= size()) {
			return npos;
		}

		const size_t pos = std::find_if(this->cbegin() + start_pos, this->cend(), p) - this->cbegin();
		return pos < this->size() ? pos : npos;
	}

	StringView substr_sentinel(size_t offset, char sentinel) const
	{
		return substr(offset, this->find(sentinel, offset));
	}

	template <class P>
	StringView substr_predicate(size_t offset, P p) const
	{
		return substr(offset, this->find_if(p, offset));
	}

	friend int compare(StringView l, StringView r);
	friend std::ostream& operator<<( std::ostream& out, const StringView string );

	bool isValid() const { return _start != nullptr; }

protected:
	friend class ArrayViewAdaptor<const char, StringView>;
	constexpr size_type		_arrayView_size() const { return _size; }
	constexpr const_pointer _arrayView_data() const { return _start; }

	const char* _start = nullptr;
	size_type   _size = 0;
};

static_assert(sizeof(StringView) == sizeof(char*) + sizeof(std::size_t), "");

inline int compare(StringView l, StringView r)
{
	if ((l._start == r._start) && (l.size() == l.size())) {
		return 0;
	}
	int ret = StringView::traits_type::compare(l.cbegin(), r.cbegin(), std::min(l.size(), r.size()));

	if (ret == 0) {
		// couldn't find a difference yet -> compare sizes
		if (l.size() < r.size()) {
			ret = -1;
		} else if (l.size() > r.size()) {
			ret = 1;
		}
	}
	return ret;
}

/* operator overloads */
// clang-format off
inline bool operator==(const StringView& l, const StringView& r) { return compare(l, r) == 0; }
inline bool operator!=(const StringView& l, const StringView& r) { return !(l == r); }
inline bool operator< (const StringView& l, const StringView& r) { return compare(l, r) < 0; }
inline bool operator> (const StringView& l, const StringView& r) { return r<l; }
inline bool operator<=(const StringView& l, const StringView& r) { return !(l>r); }
inline bool operator>=(const StringView& l, const StringView& r) { return !(l < r); }
// clang-format on
constexpr StringView EmptyStringView{ "" };
namespace _impl {
	constexpr StringView ViewOfSpaces{ "                                                                                                                                           " };
}
constexpr StringView getSpaces(size_t count)
{
	return _impl::ViewOfSpaces.substr(0, count);
}

}

namespace std {
template <>
struct hash<mart::StringView> {
	// form http://stackoverflow.com/questions/24923289/is-there-a-standard-mechanism-to-retrieve-the-hash-of-a-c-string
	std::size_t operator()(mart::StringView str) const noexcept
	{
		std::size_t h = 0;

		for (auto c : str) {
			h += h * 65599 + c;
		}

		return h ^ (h >> 16);
	}
};
}

namespace mart {

namespace details_to_integral {
//Core logic.
//Assuming number starts at first character and string is supposed to be parsed until first non-digit
template<class T>
T core(mart::StringView str)
{
	T tmp = 0;
	for (auto c : str) {
		int d = c - '0';
		if (d < 0 || 9 < d ) {
			break;
		}
		if( tmp >= std::numeric_limits<T>::max() / 16 ) { // quick check against simple constant
			if( tmp > ( std::numeric_limits<T>::max() - d ) / 10 ) {
#ifdef __cpp_rtti
				throw std::out_of_range( "String representing an integral (\"" + str.to_string() + "\") overflows type "
										 + typeid( T ).name() );

#else
				throw std::out_of_range( "String representing an integral (\"" + str.to_string()
										 + "\") overflows type in function" + __func__ );

#endif
			}
		}
		tmp = tmp * 10 + d;

	}
	return tmp;
}

//for unsigned types
template<class T>
auto base(const mart::StringView str) -> mart::enable_if_t<std::is_unsigned<T>::value, T>
{
	assert(str.size() > 0);
	if (str[0] == '+') {
		return details_to_integral::core<T>(str.substr(1));
	} else {
		return details_to_integral::core<T>(str);
	}
}

//for signed types
template<class T>
auto base(const mart::StringView str) -> mart::enable_if_t<std::is_signed<T>::value, T>
{
	assert(str.size() > 0);
	switch(str[0]) {
		case '-' : return -details_to_integral::core<T>(str.substr(1));
		case '+' : return  details_to_integral::core<T>(str.substr(1));
		default  : return  details_to_integral::core<T>(str);
	}
}
}

template<class T = int>
T to_integral(const mart::StringView str) {
	if (str.size() == 0) {
		return T{};
	}
	return details_to_integral::base<T>(str);
}

//minimal implementation for sanitized strings that only contain digits (no negative numbers)
template<class T = unsigned int>
constexpr T to_integral_unsafe( std::string_view str )
{
	T value {};
	for( auto c : str ) {
		value = value * 10 + c - '0';
	}
	return value;
}
}

#endif //LIB_MART_COMMON_GUARD_STRING_VIEW_H
