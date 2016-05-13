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

template<class T>
class ArrayView {
public:
	//type defs
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference =T&;
	using const_reference = typename std::add_const<reference>::type;
	using pointer = T*;
	using const_pointer = const T*;
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

	constexpr ArrayView(pointer other, size_t size) noexcept :
		_start(other),
		_size(size)
	{}

	template<class IT, class = enable_if_random_it_t<IT>> //FIXME: random iterator is not good enough (e.g. deque), we need c++17 contiguous iterator
	constexpr ArrayView(IT start,IT end) noexcept :
		_start(&*start),
		_size(end-start)
	{}

	template<size_t N  >
	constexpr ArrayView(T(&other)[N]) noexcept :
		_start(other),
		_size(N)
	{}

	template <class U, class = typename std::enable_if< !std::is_const<T>::value && is_compatible_container<U>::value >::type>
	constexpr ArrayView(U& other) noexcept :
		_start(other.data()),
		_size(other.size())
	{}

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
		return {reinterpret_cast<transfer_constness_t<T,uint8_t>*>(_start),size_inBytes()};
	}
	constexpr ArrayView<const uint8_t> asConstBytes() 	const { return {reinterpret_cast<const uint8_t*>(_start), size_inBytes()}; }
	constexpr ArrayView<const uint8_t> asBytes() 		const { return asConstBytes(); }


	constexpr ArrayView<T> subview(size_t offset, size_t count) const {
		//return {_start + offset,count};
		return offset + count <= this->_size ? ArrayView<T>{ this->_start + offset, count } :
			throw std::out_of_range(std::string("Tried to create a subview that would exceed the original array view.") +
				"Original size: " + std::to_string(_size) +
				". Offset/Count:" + std::to_string(offset) + "/" + std::to_string(count) + "\n");
	}

	constexpr ArrayView<T> subview(size_t offset) const {
		//return {_start + offset,count};
		return offset <= this->_size ? ArrayView<T>{ this->_start + offset, _size-offset } :
			throw std::out_of_range(std::string("Tried to create a subview that would exceed the original array view.") +
				"Original size: " + std::to_string(_size) +
				". Offset/Count:" + std::to_string(offset) + "/" + std::to_string(_size-offset) + "\n");
	}

protected:
	pointer _start = nullptr;
	size_type _size = 0;
};



}//namespace mart

#endif /* SRC_UTILS_ARRAYVIEW_H_ */
