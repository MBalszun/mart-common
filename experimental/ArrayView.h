/*
 * ArrayView.h
 *
 *  Created on: Jan 15, 2016
 *      Author: balszun
 */

#ifndef SRC_UTILS_ARRAYVIEW_H_
#define SRC_UTILS_ARRAYVIEW_H_
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <stdexcept>
#include <cstddef>
#include <cassert>

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <string>
#include <array>

/* Proprietary Library Includes */

/* Project Includes */

namespace mart {

template<class T>
class ArrayView;

using MemoryView = ArrayView<uint8_t>;
using ConstMemoryView = ArrayView<const uint8_t>;

/**
 * Class that represents a range of elements, that are stored consecutive in memory,
 * like an array or the elements inside an std::vector.
 *
 * Essentially it is a wrapper around a pointer to first element and a length and
 * was inspired by (but is not compatible to) array_view proposed for the standard
 * and the gsl::span form the c++ guidelines support library.
 *
 * Purpose/Rational:
 * It is supposed to server mainly as a generic function parameter that abstracts
 * away the difference between data that is stored in a c-style array, a std::array
 * a std::vector and similar non-standard containers but keeps the relation between
 * the pointer and the size, which makes it easier and safer to pass it around than
 * a the individual pointer / size variables. It also provides facilities for direct
 * access to the underlying storage as an array of bytes
 *
 * Contrary to pointers, and similar to c++ containers, constness on ArrayView is transitive,
 * meaning that const ArrayView<T> only allows non-modifying access to the referenced elements.
 * Similar to pointers, but contrary to some c++ containers, it is possible to declare a
 * ArrayView<const T> variable, that can e.g. be reassigned, but can't be used to modify the
 * referenced values.
 *
 * An ArrayView<T> can be implicitly converted to a ArrayView<const T>. So the default function
 * parameter type for ranges that are not modified by the function should be ArrayView<const T>:
 *
 * void foo (		ArrayView<const T> range)  //or
 * void foo (const  ArrayView<const T> range)
 *
 * Here is an overview over the different types:
 * 1)       ArrayView<      T> v1 : A non-const ArrayView variable providing read/write access to the elements
 * 2)       ArrayView<const T> v2 : A non-const ArrayView variable providing only read access to the elements
 * 3) const ArrayView<      T> v3 : A const ArrayView object that only provides read access to the elements
 * 4) const ArrayView<const T> v4 : Although the type is distinct, it is supposed to behaves the same as v3
 *
 *
 *
 */
template<class T>
class ArrayView {
public:
	static_assert(std::is_reference<T>::value == false,"T must not be a reference type");
	//The usual type defs for c++ container
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T&;
	using const_reference = typename std::add_const<reference>::type;
	using pointer = T*;
	using const_pointer = typename std::add_const<pointer>::type; //const T*;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
	/* ### Type traits short hands ### */
	template<class IT>
	struct is_random_it {
		static constexpr bool value =	std::is_same< typename std::iterator_traits<IT>::iterator_category,	std::random_access_iterator_tag	>::value;
	};

	template<class U, class = typename U::iterator>
	struct has_random_it {
		static constexpr bool value = is_random_it<typename U::iterator>::value;
	};

	template<class IT>
	using enable_if_random_it_t= typename std::enable_if<is_random_it<IT>::value>::type;

	template<class U>
	using remove_c_t = typename std::remove_const<U>::type;

	template<class U, class = typename U::iterator>
	struct is_compatible_container {
		static constexpr bool value = has_random_it<U>::value &&
				(std::is_same<typename U::value_type, remove_c_t<value_type> >::value ||
				 std::is_same<typename U::value_type, value_type >::value);
	};

	template<class U, class K>
	struct transfer_constness {
		using type = typename std::conditional<std::is_const<U>::value, typename std::add_const<K>::type, K>::type;
	};

	template<class U, class K>
	using transfer_constness_t = typename transfer_constness<U,K>::type ;

public:

	/* #### CTORS #### */
	constexpr ArrayView() = default;

	//construction from a pointer and a size (c-style api)
	constexpr ArrayView(pointer other, size_t size) noexcept :
		_start(other),
		_size(size)
	{}

	/**
	 * Construction from a pair of random access iterators denoting a range
	 * (like from calling v.begin() and v.end())
	 */
	template<class IT, class = enable_if_random_it_t<IT>> //FIXME: random iterator is not good enough (e.g. deque), we need c++17 contiguous iterator
	constexpr ArrayView(IT start,IT end) noexcept :
		_start(end-start== 0 ? nullptr : &*start),
		_size(end-start)
	{}

	/**
	 * construction from a c-style array
	 */
	template<size_t N  >
	constexpr ArrayView(T(&other)[N]) noexcept :
		_start(other),
		_size(N)
	{}

	/**
	 * construction from a container, in case T is not const (using SFINAE for disambiguation with the other constructor).
	 */
	template <class U, class = typename std::enable_if< !std::is_const<T>::value && is_compatible_container<U>::value >::type>
	constexpr ArrayView(U& other) noexcept :
		_start(other.data()),
		_size(other.size())
	{}

	/**
	 * construction from a container, in case T is const (using SFINAE for disambiguation with the other constructor).
	 */
	template <class U, class = typename std::enable_if<  std::is_const<T>::value && is_compatible_container<U>::value >::type>
	constexpr ArrayView(const U& other) noexcept :
		_start(other.data()),
		_size(other.size())
	{}

	/* #### Special member functions #### */
	constexpr 		ArrayView(const ArrayView<T>& other) = default;
	ArrayView<T>& 	operator=(const ArrayView<T>& other) = default;

	//conversion to view that can't be used to change the underlying data
	operator ArrayView<const T>() const noexcept{
		return { _start,_size };
	}

	/* #### container functions #### */

					iterator begin()		  noexcept	{ return _start; }
					iterator end()			  noexcept	{ return _start + _size; }
	constexpr const_iterator cbegin()	const noexcept	{ return _start; }
	constexpr const_iterator cend()		const noexcept	{ return _start + _size;}
	constexpr const_iterator begin()	const noexcept	{ return cbegin(); }
	constexpr const_iterator end()		const noexcept	{ return cend(); }

			reverse_iterator rbegin()					{ return reverse_iterator(end()); }
			reverse_iterator rend()						{ return reverse_iterator(begin()); }
	  const_reverse_iterator crbegin()	const			{ return const_reverse_iterator(end()); }
	  const_reverse_iterator crend()	const			{ return const_reverse_iterator(begin()); }
	  const_reverse_iterator rbegin()	const			{ return crbegin(); }
	  const_reverse_iterator rend()		const			{ return crend(); }

					reference front()					{ return *begin(); }
	constexpr const_reference front()	const			{ return *begin(); }
					reference back()					{ return *(end() - 1); }
	constexpr const_reference back()	const			{ return *(end() - 1); }

	constexpr size_type size()			const noexcept	{ return _size; }
	constexpr size_type size_inBytes()	const noexcept	{ return _size*sizeof(value_type); }

	constexpr size_type length()		const noexcept	{ return size(); }
	constexpr bool empty()				const noexcept	{ return size() == 0; }

					reference operator[](size_t idx)			{ return _start[idx]; }
	constexpr const_reference operator[](size_t idx)	const	{ return _start[idx]; }

					pointer data()		 { return _start; }
	constexpr const_pointer data() const { return _start; }

	constexpr const char* asConstCharPtr() const { return reinterpret_cast<const char*>(_start);}


	/*#### view functions ####*/

	ArrayView<transfer_constness_t<T,uint8_t>> asBytes() {
		return { reinterpret_cast<transfer_constness_t<T,uint8_t>*>(_start),size_inBytes()};
	}
	constexpr ArrayView<const uint8_t> asConstBytes() 	const { return {reinterpret_cast<const uint8_t*>(_start), size_inBytes()}; }
	constexpr ArrayView<const uint8_t> asBytes() 		const { return asConstBytes(); }


	constexpr ArrayView<T> subview(size_t offset, size_t count) const {
		return  _throwIfInvalidSubview(offset, count) , //found a usage for the comma (,) operator ;)
				ArrayView<T>{ _start + offset, count};
	}

	/**
	 * Creates a subview starting at offset and ending a the same a element as the original
	 *
	 * Rational:
	 * The reason for defining the semantics of the a single parameter subview function this way,
	 * although it could cause some confusion, was that it helps more to have a shorthand for
	 *
	 * 	view.subview(offset, view.size()-offset)
	 *
	 * 		than for
	 *
	 *  view.subview(0,count)
	 */
	constexpr ArrayView<T> subview(size_t offset) const {
		return  _throwIfInvalidSubview(offset, _size-offset) ,//found a usage for the comma (,) operator ;)
				ArrayView<T>{ _start + offset, _size-offset};
	}

protected:
	pointer _start = nullptr;
	size_type _size = 0;

	constexpr bool _throwIfInvalidSubview(size_t offset, size_t count ) const {
		//this will trigger a compile time error, when called in a constexpr context
		//XXX: nasty hack:
		//c++11 constexpr functions need to have a return type other than void, although we are not interested in it here
		return offset + count <= _size ?
				true :
				throw std::out_of_range(std::string("Tried to create a subview that would exceed the original array view.") +
						"Original size: " + std::to_string(_size) +
						". Offset/Count:" + std::to_string(offset) + "/" + std::to_string(count) + "\n");

	}
};



}//namespace mart

#endif /* SRC_UTILS_ARRAYVIEW_H_ */
