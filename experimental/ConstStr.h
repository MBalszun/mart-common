/*
 * ConstStr.h
 *
 *  Created on: Jun 20, 2016
 *      Author: balszun
 */

#ifndef LIBS_MART_COMMON_EXPERIMENTAL_CONSTSTR_H_
#define LIBS_MART_COMMON_EXPERIMENTAL_CONSTSTR_H_
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <memory>

/* Proprietary Library Includes */

/* Project Includes */
#include "StringView.h"

namespace mart {

class ConstStr : public str_ref {

public:
	/* #### CTORS #### */
	constexpr ConstStr() = default;

	ConstStr(const char* other, size_t size) 	{ _copyFrom(other, size); }
	explicit ConstStr(const std::string& other) { _copyFrom(other.c_str(), other.size()); }
	explicit ConstStr(const str_ref& other) 	{ _copyFrom(other.cbegin(), other.size()); }

	// don't accept c-strings in the form of pointer
	// if you need to create a const_string from a c string use the <const_string(const char*, size_t)> constructor
	// if you need to create a ConstStr form a string literal, use the <ConstStr(const char(&other)[N])> constructor
	template<class T>
	ConstStr(T const * const& other) = delete;

	//NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<size_t N>
	explicit constexpr ConstStr(const char(&other)[N]) noexcept :
		str_ref(other, N - 1)
	{}

	/* #### Special member functions #### */
	ConstStr(const ConstStr& other) = default;
	ConstStr(ConstStr&& other) = default;
	ConstStr& operator=(const ConstStr& other) = default;
	ConstStr& operator=(ConstStr&& other) = default;

	/* #### String functions  #### */
	ConstStr substr(size_t offset, size_t count) const {
		ConstStr retval;
		static_cast<str_ref&>(retval) = str_ref::substr(offset, count);
		retval._data = this->_data;
		return retval;
	}

	template<class ...ARGS>
	friend ConstStr concat(ARGS&&...args);

private:
	std::shared_ptr<char> _data = nullptr;

	void _copyFrom(const char* other, size_t size) {
		_data = std::shared_ptr<char>(new char[size], std::default_delete<char[]>());
		std::copy_n(other, size, _data.get());
		_size = size; // last element is null-character and not counted to size
		_start = _data.get();
	}

	//impl helper for concat
	static void _addTo(char*& buffer, const str_ref& str) {
		std::copy_n(str.cbegin(), str.size(), buffer);
		buffer += str.size();
	}

	template<class ...ARGS>
	static ConstStr _concat_impl(const ARGS& ...args) {
		//determine total size
		size_t newSize = 0;
		int ignore[] = { (newSize += args.size(),0)... };
		(void)ignore;

		//create const_string object
		ConstStr retval;
		retval._data = std::shared_ptr<char>(new char[newSize], std::default_delete<char[]>());
		retval._start = retval._data.get();
		retval._size = newSize;

		//place copy arguments to buffer
		char * bufferStart = retval._data.get();
		int ignore2[] = { (_addTo(bufferStart,args),0)... };
		(void)ignore2;
		return retval;
	}
};

template<class ...ARGS>
ConstStr concat(ARGS&&...args) {
	return ConstStr::_concat_impl(str_ref(args)...);
}

}



#endif /* LIBS_MART_COMMON_EXPERIMENTAL_CONSTSTR_H_ */
