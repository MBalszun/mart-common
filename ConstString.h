#ifndef LIBS_MART_COMMON_CONST_STRING_H
#define LIBS_MART_COMMON_CONST_STRING_H
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <cassert>

/* Proprietary Library Includes */
#include "./cpp_std/utility.h"
#include "./cpp_std/memory.h"

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

	//NOTE: Use only for string literals (arrays with static storage duration)!!!
	template<size_t N>
	constexpr ConstString(const char(&other)[N]) noexcept :
	StringView(other)
		//we don't have to initialize the shared_ptr to anything as string litterals already have static lifetime
	{
		static_assert(N >= 1, "");
	}

	// don't accept c-strings in the form of pointer
	// if you need to create a const_string from a c string use the <const_string(const char*, size_t)> constructor
	// if you need to create a ConstString form a string literal, use the <ConstString(const char(&other)[N])> constructor
	template<class T>
	ConstString(T const * const& other) = delete;

protected:
	/** private constructor, that takes ownership of a buffer and a size (used in _copyFrom and _concat_impl)
	*
	* Implementation notes:
	* - creating from shared_ptr doesn't bring any advantage, as you can't use std::make_shared for an array
	* - This automatically stores the correct deleter in the shared_ptr (by default shared_ptr<const char> would use delete instead of delete[]
	*/
	ConstString(std::unique_ptr<const char[]> data, size_t size) :
		StringView(data.get(), size),
		_data(std::move(data))
	{
		assert(_start != nullptr);
	}
public:

	/* ############### Special member functions ######################################## */
	ConstString(const ConstString& other) noexcept = default;
	ConstString& operator=(const ConstString& other) noexcept = default;

	ConstString(ConstString&& other) noexcept
		: StringView(mart::exchange(other._as_strview(), mart::EmptyStringView))
		, _data(std::move(other._data))
	{
	}
	ConstString& operator=(ConstString&& other) noexcept
	{
		this->_as_strview() = mart::exchange(other._as_strview(), mart::EmptyStringView);
		_data = std::move(other._data);
		return *this;
	}

	/* ################## String functions  ################################# */
	//ConstString substr(size_t offset, size_t count) const
	//{
	//	ConstString retval;
	//	//use substr fucntionality from our base class
	//	// and copy pointer underlying storage
	//	retval._as_strview() = this->_as_strview().substr(offset, count);
	//	retval._data = this->_data;
	//	return retval;
	//}

	//ConstString substr(size_t offset) const
	//{
	//	return substr(offset, _size - offset);
	//}

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

	const char* c_str() const {
		if (!isZeroTerminated()) {
			throw std::exception("Called c_str on ConstString that is not zero terminated -> create zero terminated version wihth createZStr() first");
		}
		return _start;
	}

	template<class ...ARGS>
	friend ConstString concat(const ARGS&...args);

	template<class ...ARGS>
	friend std::string concat_cpp_str(const ARGS&...args);

private:
	std::shared_ptr<const char> _data = nullptr;

	friend void swap(ConstString& l, ConstString& r)
	{
		using std::swap;
		swap(l._as_strview(), r._as_strview());
		swap(l._data, r._data);
	}

	StringView& _as_strview()
	{
		return static_cast<StringView&>(*this);
	}

	const StringView& _as_strview() const
	{
		return static_cast<const StringView&>(*this);
	}

	static inline std::unique_ptr<char[]> _allocate_null_terminated_char_buffer(size_t size)
	{
		auto data = mart::make_unique<char[]>(size + 1);
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
	inline static size_t _total_size(const ARGS& ...args)
	{
		//c++17: ~ const size_t newSize = 0 + ... + args.size();
		size_t newSize = 0;
		const int ignore1[] = { (newSize += args.size(),0)... };
		(void)ignore1;
		return newSize;
	}

	template<class ...ARGS>
	inline static void _write_to_buffer(char* buffer, const ARGS& ...args)
	{
		const int tignore[] = { (_addTo(buffer,args),0)... };
		(void)tignore;
	}

	template<class ...ARGS>
	inline static ConstString _concat_impl(const ARGS& ...args)
	{
		const size_t newSize = _total_size(args ...);

		auto data = _allocate_null_terminated_char_buffer(newSize);

		_write_to_buffer(data.get(), args ...);

		return ConstString(std::move(data), newSize);
	}

};


/**
* Function that can concatenate an arbitrary number of objects from which a mart::string_view can be constructed
* returned constStr will always be zero terminated
*/
template<class ...ARGS>
ConstString concat(const ARGS& ...args)
{
	return ConstString::_concat_impl(StringView(args)...);
}

template<class ...ARGS>
std::string concat_cpp_str(const ARGS& ...args)
{
	const size_t newSize = ConstString::_total_size(StringView(args) ...);

	std::string ret(newSize, ' ');

	ConstString::_write_to_buffer(&ret[0], StringView(args) ...);

	return ret;
}

inline const mart::ConstString& getEmptyConstString()
{
	const static mart::ConstString str(mart::EmptyStringView);
	return str;
}

}

#endif