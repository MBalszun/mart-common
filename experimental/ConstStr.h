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

/**
 * ConstStr is a ref-counted String implementation, that doesn't allow the modification of the underlying storage at all.
 *
 * One particular property is that when it is constructed from a "const char [N]" argument it is assumed, that this represents
 * a string litteral, in which case ConstStr doesn't perform any copy or dynamic memory allocation and also
 * copying the ConstStr will not result in any copyies or refcount updates.
 *
 * This header also provides a function that can efficently concatenate multiple string like objects, because it
 * needs only a single dynamic memory allocation
 *
 */
class ConstStr : public string_view {

public:
	/* #### CTORS #### */
	constexpr ConstStr() = default;

	ConstStr(string_view other)
	{
		_copyFrom(other);
	}
	ConstStr(const char* other, size_t size) :
		ConstStr{ string_view{other,size} }
	{}
	explicit ConstStr(const std::string& other) :
		ConstStr{ string_view(other) }
	{}

	// don't accept c-strings in the form of pointer
	// if you need to create a const_string from a c string use the <const_string(const char*, size_t)> constructor
	// if you need to create a ConstStr form a string literal, use the <ConstStr(const char(&other)[N])> constructor
	template<class T>
	ConstStr(T const * const& other) = delete;

	//NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<size_t N>
	constexpr ConstStr(const char(&other)[N]) noexcept :
		string_view(other)
		//we don't have to initialize the shared_ptr to anything as string litterals already have static lifetime
	{}

	/* #### Special member functions #### */
	ConstStr(const ConstStr& other) = default;
	ConstStr(ConstStr&& other) = default;
	ConstStr& operator=(const ConstStr& other) = default;
	ConstStr& operator=(ConstStr&& other) = default;

	/* #### String functions  #### */
	ConstStr substr(size_t offset, size_t count) const
	{
		ConstStr retval;
		//use substr fucntionality from our base class
		static_cast<string_view&>(retval) = string_view::substr(offset, count);
		//copy underlying storage
		retval._data = this->_data;
		return retval;
	}

	/* #### String functions  #### */
	ConstStr substr(size_t offset) const
	{
		return substr(offset, _size - offset);
	}

	template<class ...ARGS>
	friend ConstStr concat(ARGS&&...args);

private:
	std::shared_ptr<const char> _data = nullptr;

	void _copyFrom(const mart::string_view other)
	{
		//create buffer and copy data over
		auto data = std::unique_ptr<char[]>(new char[other.size()]); //c++14: auto data= std::make_unique<char[]>(size);
		std::copy_n(other.data(), other.size(), data.get());

		//initialize ConstStr data fields;
		*this = ConstStr(std::move(data), other.size());
	}

	//private constructor, that takes ownership of a buffer and a size (used in _copyFrom and _concat_impl)
	ConstStr(std::unique_ptr<const char[]> data, size_t size) :
		string_view(data.get(), size),
		_data(std::move(data))
	{}

	//######## impl helper for concat ###############
	static void _addTo(char*& buffer, const string_view& str)
	{
		std::copy_n(str.cbegin(), str.size(), buffer);
		buffer += str.size();
	}

	template<class ...ARGS>
	inline static ConstStr _concat_impl(const ARGS& ...args)
	{
		//determine required size
		//c++17: ~ const size_t newSize = 0 + ... + args.size();
		//const size_t newSize = [&]() { //can't use lambdas here due to bug in g++ 4.8
		size_t newSize = 0;
		const int ignore1[] = { (newSize += args.size(),0)... };
		(void)ignore1;
		//	return newSize;
		//}();

		//construct buffer and copy data
		//auto data = [&]() { //can't use lambdas here due to bug in g++ 4.8
		auto data = std::unique_ptr<char[]>(new char[newSize]);
		char * bufferStart = data.get();
		const int ignore2[] = { (_addTo(bufferStart,args),0)... };
		(void)ignore2;
		//	return data;
		//}();

		return ConstStr(std::move(data),newSize	);
	}
};

/**
 * Function that can concatenate an arbitrary number of objects from which a mart::string_view can be constructed
 */
template<class ...ARGS>
ConstStr concat(ARGS&&...args)
{
	return ConstStr::_concat_impl(string_view(std::forward<ARGS>(args))...);
}

}



#endif /* LIBS_MART_COMMON_EXPERIMENTAL_CONSTSTR_H_ */
