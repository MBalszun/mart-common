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
#include <memory>
#include <type_traits>
#include <string>

/* Proprietary Library Includes */

/* Project Includes */
#include "ArrayViewAdaptor.h"

namespace mart {

class str_ref : public ArrayViewAdaptor<const char,str_ref>{
public:
	//type defs
	using CharT = char;
	using traits_type = std::char_traits<CharT>;

	static constexpr size_type npos = size_type(-1);

public:
	/* #### CTORS #### */
	constexpr str_ref() = default;

	str_ref(const std::string& other) noexcept:
		_start(other.data()),
		_size(other.size())
	{}

	constexpr str_ref(const char* other, size_type size) noexcept :
		_start(other),
		_size(size)
	{}

	//NOTE: Use only for string literals!!!
	template<size_t N>
	constexpr str_ref(const char(&other)[N]) noexcept :
		_start(other),
		_size(N - 1)
	{}

	template<class T>
	str_ref(const T * const& other) = delete;

	/* #### Special member functions #### */
	constexpr str_ref(const str_ref& other) = default;
	str_ref& operator=(const str_ref& other) = default;

	/*#### string functions ####*/
	std::string to_string() const {
		return std::string(cbegin(), cend());
	}

	constexpr str_ref substr(size_t offset, size_t count) const {
		return offset + count <= this->_size ? str_ref{ this->_start + offset, count } :
			throw std::out_of_range("Tried to create a substring that would exceed the original string. Original string:\n\n" + this->to_string() + "\"\n");
	}



	/*#### algorithms ####*/

	size_type find(str_ref str, size_type pos = 0) const {
		if (pos >= str.size()) return npos;
		auto it = std::search(this->cbegin()+pos, this->cend(), str.cbegin(), str.cend());
		return it != this->cend() ? it - this->cbegin() : npos;
	}

	friend int compare(str_ref l, str_ref r);
	friend std::ostream& operator<<(std::ostream& out, const str_ref& string) {
		out.write(string.data(), string.size());
		return out;
	}
protected:
	using ArrayViewAdaptor<const char, str_ref>::data;

	friend class ArrayViewAdaptor<const char, str_ref>;
	constexpr	  size_type _arrayView_size() const { return _size; }
	constexpr const_pointer _arrayView_data() const { return _start; }

	const char* _start = nullptr;
	size_type _size = 0;
};

inline int compare(str_ref l, str_ref r) {
	if ((l._start == r._start) && (l.size() == l.size())) {
		return 0;
	}
	int ret = str_ref::traits_type::compare(l.cbegin(), r.cbegin(), std::min(l.size(), r.size()));

	if (ret == 0) {
		//couldn't find a difference yet -> compare sizes
		if (l.size() < r.size()) {
			ret = -1;
		} else if (l.size() > r.size()) {
			ret = 1;
		}
	}
	return ret;
}

/* operator overloads */
inline bool operator==(const str_ref& l, const str_ref& r) { return compare(l,r) == 0; }
inline bool operator!=(const str_ref& l, const str_ref& r) { return !(l == r); }
inline bool operator< (const str_ref& l, const str_ref& r) { return compare(l,r) < 0; }
inline bool operator> (const str_ref& l, const str_ref& r) { return r<l; }
inline bool operator<=(const str_ref& l, const str_ref& r) { return !(l>r); }
inline bool operator>=(const str_ref& l, const str_ref& r) { return !(l < r); }



}

namespace std {
template <> struct hash<mart::str_ref>
{
	size_t operator()(const mart::str_ref & x) const
	{
		//XXX some arbitrarily put together hashing algorithm
		auto hash = std::hash<char>{};
		size_t accum = 0;
		for (auto c : x) {
			auto tmp = hash(c);
			tmp = (tmp << 6) + (tmp >> 2);
			accum ^= tmp;
		}
		return accum;
	}
};
}



#endif /* LIBS_MART_COMMON_EXPERIMENTAL_STRINGVIEW_H_ */
