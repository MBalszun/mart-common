/*
 * ConstString.h
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
#include <cassert>
#include "../cpp_std/utility.h"
#include "../cpp_std/memory.h"

/* Proprietary Library Includes */

/* Project Includes */
#include "StringView.h"

namespace mart {

/**
 * ConstString is a ref-counted String implementation, that doesn't allow the modification of the underlying storage at all.
 *
 * One particular property is that when it is constructed from a "const char [N]" argument it is assumed, that this represents
 * a string litteral, in which case ConstString doesn't perform any copy or dynamic memory allocation and also
 * copying the ConstString will not result in any copyies or refcount updates.
 *
 * This header also provides a function that can efficently concatenate multiple string like objects, because it
 * needs only a single dynamic memory allocation
 *
 */
class ConstString : public StringView {
public:
	/* #################### CTORS ########################## */
	//Default ConstString points at empty string
	ConstString() :
		StringView(mart::EmptyStringView)
	{};

	explicit ConstString(StringView other)
	{
		_copyFrom(other);
	}

	// don't accept c-strings in the form of pointer
	// if you need to create a const_string from a c string use the <const_string(const char*, size_t)> constructor
	// if you need to create a ConstString form a string literal, use the <ConstString(const char(&other)[N])> constructor
	template<class T>
	ConstString(T const * const& other) = delete;

	//NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<size_t N>
	ConstString(const char(&other)[N]) noexcept :
		StringView(other)
		//we don't have to initialize the shared_ptr to anything as string litterals already have static lifetime
	{
		static_assert(N >= 1, "");
	}
protected:
	//private constructor, that takes ownership of a buffer and a size (used in _copyFrom and _concat_impl)
	ConstString(std::unique_ptr<const char[]> data, size_t size) :
		StringView(data.get(), size),
		_data(std::move(data))
	{
		assert(_start != nullptr);
	}
public:

	/* ############### Special member functions ######################################## */
	ConstString(const ConstString& other) = default;
	ConstString& operator=(const ConstString& other) = default;

	ConstString(ConstString&& other):
		//string_view{ nullptr,other.size() },
		StringView(other),
		_data(std::move(other._data))
	{
		other._as_strview() = mart::EmptyStringView;
	}
	ConstString& operator=(ConstString&& other)
	{
		this->_as_strview() = mart::exchange(other._as_strview(), mart::EmptyStringView);
		_data = std::move(other._data);
		return *this;
	}

	/* ################## String functions  ################################# */
	ConstString substr(size_t offset, size_t count) const
	{
		ConstString retval;
		//use substr fucntionality from our base class
		// and copy pointer underlying storage
		retval._as_strview() = retval._as_strview().substr(offset, count);
		retval._data = this->_data;
		return retval;
	}

	ConstString substr(size_t offset) const
	{
		return substr(offset, _size - offset);
	}

	bool isZeroTerminated() const
	{
		return (*this)[size()] == '\0';
	}

	ConstString unshare() const
	{
		return ConstString(static_cast<mart::StringView>(*this));
	}

	ConstString createZStr() const &
	{
		if (isZeroTerminated()) {
			return *this; //just copy
		} else {
			return unshare();
		}
	}

	ConstString createZStr() &&
	{
		if (isZeroTerminated()) {
			return std::move(*this); //already zero terminated - just move
		} else {
			return unshare();
		}
	}

	template<class ...ARGS>
	friend ConstString concat(ARGS&&...args);

private:
	std::shared_ptr<const char> _data = nullptr;

	StringView& _as_strview()
	{
		return static_cast<StringView&>(*this);
	}

	static inline std::unique_ptr<char[]> _allocate_null_terminated_char_buffer(size_t size)
	{
		auto data = mart::make_unique<char[]>(size + 1);//std::unique_ptr<char[]>(new char[size + 1]); //c++14: auto data= std::make_unique<char[]>(size+1);
		data[size] = '\0'; //zero terminate
		return data;
	}

	void _copyFrom(const mart::StringView other)
	{
		if (other.data() == nullptr) {
			this->_as_strview() = EmptyStringView;
			return;
		}
		//create buffer and copy data over
		auto data = _allocate_null_terminated_char_buffer(other.size());
		std::copy_n(other.data(), other.size(), data.get());

		//initialize ConstString data fields;
		*this = ConstString(std::move(data), other.size());
	}

	//######## impl helper for concat ###############
	static void _addTo(char*& buffer, const StringView& str)
	{
		std::copy_n(str.cbegin(), str.size(), buffer);
		buffer += str.size();
	}

	template<class ...ARGS>
	inline static ConstString _concat_impl(const ARGS& ...args)
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
		auto data = _allocate_null_terminated_char_buffer(newSize);
		char * bufferStart = data.get();
		const int ignore2[] = { (_addTo(bufferStart,args),0)... };
		(void)ignore2;
		//	return data;
		//}();

		return ConstString(std::move(data),newSize	);
	}
};

/**
 * Function that can concatenate an arbitrary number of objects from which a mart::string_view can be constructed
 * returned constStr will always be zero terminated
 */
template<class ...ARGS>
ConstString concat(ARGS&&...args)
{
	return ConstString::_concat_impl(StringView(std::forward<ARGS>(args))...);
}

}



#endif /* LIBS_MART_COMMON_EXPERIMENTAL_CONSTSTR_H_ */
