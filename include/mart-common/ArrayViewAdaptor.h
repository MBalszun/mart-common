#ifndef LIB_MART_COMMON_GUARD_ARRAY_VIEW_ADAPTOR_H
#define LIB_MART_COMMON_GUARD_ARRAY_VIEW_ADAPTOR_H
/**
 * ArrayViewAdaptor.h (mart-common)
 *
 * Copyright (C) 2016-2020: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  mart::ArrayViewAdaptor, a crtp-style mixin to generate memberfunctions similar to array view from a pointer
 * and a length
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
/* Proprietary Library Includes */
/* Standard Library Includes */
#include <cstddef>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

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
 * 			Foo(size_t size)
 * 			    : _data(new int[size])
 * 				, _size(size)
 * 			{
 * 				std::fill_n(_data.get(),size,0);
 * 			}
 *
 * 		private:
 * 			size_t _size;
 * 		 	unique_ptr<int[]> _data;
 *
 * 		 	friend class ArrayViewAdaptor<int,Foo>;
 * 				  int* _arrayView_data()       { return _data.get(); }
 * 		    const int* _arrayView_data() const { return _data.get(); }
 * 			    size_t _arrayView_size() const { return _size; }
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
template<class T, class DerivedType>
class ArrayViewAdaptor {
public:
    // clang format doesn't support alignment of individual parts of a declaration/definition
	// clang-format off

	//The usual type defs for c++ container
	using value_type      = T;
	using size_type       = std::size_t;
	using difference_type = std::ptrdiff_t;
	using       reference =       value_type&;
	using const_reference = const value_type&;
	using       pointer   =       value_type*;
	using const_pointer   = const value_type*; //const T*;
	using       iterator  =       pointer;
	using const_iterator  = const_pointer;

	/* #### container functions #### */
	[[nodiscard]] constexpr       iterator begin()         noexcept { return _data(); }
	[[nodiscard]] constexpr       iterator end()           noexcept { return _data() + _size(); }
	[[nodiscard]] constexpr const_iterator cbegin()  const noexcept { return _data(); }
	[[nodiscard]] constexpr const_iterator cend()    const noexcept { return _data() + _size(); }
	[[nodiscard]] constexpr const_iterator begin()   const noexcept { return cbegin(); }
	[[nodiscard]] constexpr const_iterator end()     const noexcept { return cend(); }

	[[nodiscard]] constexpr	      reference front()        noexcept { return *begin(); }
	[[nodiscard]] constexpr const_reference front()  const noexcept { return *begin(); }
	[[nodiscard]] constexpr		  reference back()         noexcept { return *( end() - 1 ); }
	[[nodiscard]] constexpr const_reference back()   const noexcept { return *( end() - 1 ); }

    [[nodiscard]] constexpr       reference operator[]( size_t idx )       noexcept { return _data()[idx]; }
	[[nodiscard]] constexpr const_reference operator[]( size_t idx ) const noexcept { return _data()[idx]; }

	/* #### container functions #### */
	[[nodiscard]] constexpr            bool empty() const noexcept { return _size() == 0; }
	[[nodiscard]] constexpr       size_type size()  const noexcept { return _size(); }
	[[nodiscard]] constexpr         pointer data()        noexcept { return _data(); }
	[[nodiscard]] constexpr   const_pointer data()  const noexcept { return _data(); }

protected:
	//special member functions are protected
	//in order to prevent ArrayViewAdaptor to be instantiated on it's own
	constexpr ArrayViewAdaptor() noexcept	= default;

private:
	[[nodiscard]] constexpr       pointer _data()       noexcept { return static_cast<      DerivedType*>(this)->_arrayView_data(); }
	[[nodiscard]] constexpr const_pointer _data() const noexcept { return static_cast<const DerivedType*>(this)->_arrayView_data(); }
	[[nodiscard]] constexpr     size_type _size() const noexcept { return static_cast<const DerivedType*>(this)->_arrayView_size(); }
	// clang-format on
};

} // namespace mart

#endif
