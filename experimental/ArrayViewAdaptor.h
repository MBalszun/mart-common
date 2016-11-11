#pragma once
/*
 * ArrayViewAdaptor.h
 *
 *  Created on: Jun 20, 2016
 *      Author: balszun
 */

#ifndef LIBS_MART_COMMON_EXPERIMENTAL_ARRAYVIEWADAPTOR_H_
#define LIBS_MART_COMMON_EXPERIMENTAL_ARRAYVIEWADAPTOR_H_

/* ######## INCLUDES ######### */
/* Standard Library Includes */

/* Proprietary Library Includes */

/* Project Includes */

#include <iterator>
#include <type_traits>
#include <stdexcept>
#include <string>

namespace mart {
/**
 * CRTP- Type class that can be used to provide standard array view functions by inheriting from it
 *
 * For this you have to
 * - inherit form it
 * - provide 3 functions:
 * 	-       T* _arrayView_data()
 * 	- const T* _arrayView_data() const
 * 	- size_t   _arrayView_size() const;
 *
 * 	e.g.:
 *
 * 		class Foo : public ArrayViewAdaptor<int,Foo> {
 * 		public:
 * 			Foo(size_t size) :
 * 				_data(new int[size]),
 * 				_size(size)
 * 			{
 * 				std::fill_n(_data.get(),size,0);
 * 			}
 *
 * 		private:
 * 			size_t _size;
 * 		 	unique_ptr<int[]> _data;
 *
 * 		 	friend class ArrayViewAdaptor<int,Foo>;
 * 				  int* _arrayView_data() {return _data.get();}
 * 		    const int* _arrayView_data() const {return _data.get();}
 * 			  size_t _arrayView_size() const {return _size;}
 * 		};
 *
 * 		//now we can do the following:
 *
 * 		Foo foo(10);
 * 		foo[3] = 5;
 * 		for (int e : foo) {
 * 			std::cout << e << std::endl;
 * 		}
 *
 */


template<class T, class DERIVED_TYPE>
class ArrayViewAdaptor {
public:
	static_assert(std::is_reference<T>::value == false, "T must not be a reference type");
	//The usual type defs for c++ container
	using value_type		= T;
	using size_type			= std::size_t;
	using difference_type	= std::ptrdiff_t;
	using		reference	=		value_type&;
	using const_reference	= const	value_type&;
	using		pointer		=		value_type*;
	using const_pointer		= const value_type*; //const T*;
	using		iterator	=		pointer;
	using const_iterator	= const_pointer;
	using		reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	/* #### container functions #### */

					iterator begin()		  noexcept { return _data(); }
					iterator end()			  noexcept { return _data() + _size(); }
	constexpr const_iterator cbegin()	const noexcept { return _data(); }
	constexpr const_iterator cend()		const noexcept { return _data() + _size(); }
	constexpr const_iterator begin()	const noexcept { return cbegin(); }
	constexpr const_iterator end()		const noexcept { return cend(); }

		  reverse_iterator rbegin()			  { return reverse_iterator(end()); }
		  reverse_iterator rend()			  { return reverse_iterator(begin()); }
	const_reverse_iterator crbegin()	const { return const_reverse_iterator(end()); }
	const_reverse_iterator crend()		const { return const_reverse_iterator(begin()); }
	const_reverse_iterator rbegin()		const { return crbegin(); }
	const_reverse_iterator rend()		const { return crend(); }

					reference front()		  { return *begin(); }
	constexpr const_reference front()	const { return *begin(); }
					reference back()		  { return *(end() - 1); }
	constexpr const_reference back()	const { return *(end() - 1); }

	constexpr size_type length()		const noexcept { return _size(); }
	constexpr bool empty()				const noexcept { return _size() == 0; }

					reference operator[](size_t idx)		{ return _data()[idx]; }
	constexpr const_reference operator[](size_t idx) const  { return _data()[idx]; }

		  reference at(size_t idx)		  { _throwIfOutOfRange(idx); return _data()[idx]; }
	const_reference at(size_t idx)	const { _throwIfOutOfRange(idx); return _data()[idx]; }

	/* #### container functions #### */
	constexpr		size_type size()	const noexcept { return _size(); }
					pointer data() 			  noexcept { return _data(); }
	constexpr const_pointer data()		const noexcept { return _data(); }

protected:
	//those special member functions are protected
	//in order to prevent ArrayViewAdaptor to be instantiated on it's own
	constexpr ArrayViewAdaptor()=default;
	~ArrayViewAdaptor() = default;
	constexpr bool _throwIfOutOfRange(size_t idx) const { return idx < _size() ? true : throw std::out_of_range("Tried to access " + std::to_string(idx) + "th element of an Array view of size" + std::to_string(_size())); }

private:
					pointer _data()       { return static_cast<		 DERIVED_TYPE*>(this)->_arrayView_data(); }
	constexpr const_pointer _data()	const { return static_cast<const DERIVED_TYPE*>(this)->_arrayView_data(); }
	constexpr	  size_type	_size()	const { return static_cast<const DERIVED_TYPE*>(this)->_arrayView_size(); }

};

}



#endif /* LIBS_MART_COMMON_EXPERIMENTAL_ARRAYVIEWADAPTOR_H_ */
