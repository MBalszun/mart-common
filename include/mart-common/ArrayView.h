#ifndef LIB_MART_COMMON_GUARD_ARRAY_VIEW_H
#define LIB_MART_COMMON_GUARD_ARRAY_VIEW_H
/**
 * ArrayView.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides ArrayView - a wrapper around a pointer and a size
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator> //iterator tags
#include <stdexcept>
#include <type_traits>
#ifndef NDEBUG
#include <string> //exception messages
#endif

/* Proprietary Library Includes */

/* Project Includes */
#include "ArrayViewAdaptor.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

/**
 * ArrayView
 *
 * @brief: Class that represents a sequence of elements stored consecutively in memory
 *
 *
 * Essentially it is a wrapper around a pointer to first element and a length and
 * was inspired by (but is not compatible to) array_view proposed for the standard
 * and the gsl::span form the c++ guidelines support library.
 *
 * Purpose/Rational:
 * It is supposed to serve mainly as a generic function parameter that abstracts
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
 * void foo (	   ArrayView<const T> range)  //or
 * void foo (const  ArrayView<const T> range)
 *
 * Here is an overview over the different types:
 * 1)       ArrayView<      T> v1 : A non-const ArrayView variable providing read/write access to the elements
 * 2)       ArrayView<const T> v2 : A non-const ArrayView variable providing only read access to the elements
 * 3) const ArrayView<      T> v3 : A const ArrayView object that only provides read access to the elements
 * 4) const ArrayView<const T> v4 : Although the type is distinct, it is supposed to behaves the same as v3
 *
 */

template<class T>
class ArrayView;

// using ByteType = std::byte;
using ByteType = uint8_t;

using MemoryView      = ArrayView<ByteType>;
using ConstMemoryView = ArrayView<const ByteType>;

namespace _detail_array_view {
/* ### Type traits short hands ### */

template<class IT>
constexpr bool is_random_it_v
	= std::is_same_v<typename std::iterator_traits<IT>::iterator_category, std::random_access_iterator_tag>;

template<class IT>
using enable_if_random_it_t = std::enable_if_t<is_random_it_v<IT>>;

template<class U, class K>
using transfer_constness_t = std::conditional_t<std::is_const<U>::value, std::add_const_t<K>, K>;

template<class U, class value_type>
constexpr auto is_compatible_container_helper( int )
	-> decltype( ( std::declval<U>().data() + std::declval<U>().size() ) == std::declval<U>().data() )
{
	return std::is_same<typename std::iterator_traits<typename U::iterator>::iterator_category,
						std::random_access_iterator_tag>::value
		   && ( std::is_same<typename U::value_type, std::remove_const_t<value_type>>::value
				|| std::is_same<typename U::value_type, value_type>::value );
};

template<class U, class value_type>
constexpr auto is_compatible_container_helper( char ) -> bool
{
	return false;
};

/* IMPLEMENTATION NOTE:
 * There are two overloads of is_compatible_container_helper:
 * one taking an argument of type nullptr_t
 * one taking an argument of type U*
 *
 * If U doesn't have the member functions data() and size(), the first overload SFINAEs away,
 * leaving only the second one which always returns false
 *
 * If U has both member functions the first one winns as nullptr_t is an exact match, whereas U* needs a
 * conversion. The first one then checks if the container has the correct value type
 *
 */
template<class U, class value_type>
constexpr bool is_compatible_container_v = is_compatible_container_helper<U, value_type>( 0 );

} // namespace _detail_array_view

template<class T>
class ArrayView : public ArrayViewAdaptor<T, ArrayView<T>> {
public:
	using value_type     = typename ArrayViewAdaptor<T, ArrayView<T>>::value_type;
	using size_type      = typename ArrayViewAdaptor<T, ArrayView<T>>::size_type;
	using pointer        = typename ArrayViewAdaptor<T, ArrayView<T>>::pointer;
	using const_pointer  = typename ArrayViewAdaptor<T, ArrayView<T>>::const_pointer;
	using iterator       = typename ArrayViewAdaptor<T, ArrayView<T>>::iterator;
	using const_iterator = typename ArrayViewAdaptor<T, ArrayView<T>>::const_iterator;

private:
	// const ByteType if T is const, ByteType otherwise
	using matching_byte_t = _detail_array_view::transfer_constness_t<T, ByteType>;
	using matching_char_t = _detail_array_view::transfer_constness_t<T, char>;

public:
	/* #### CTORS #### */
	constexpr ArrayView() noexcept = default;

	// construction from a pointer and a size (c-style api)
	constexpr ArrayView( pointer other, size_t size ) noexcept
		: _data( other )
		, _size( size )
	{
	}

	/**
	 * Construction from a pair of random access iterators denoting a range
	 * (like from calling v.begin() and v.end())
	 */
	template<class IT,
			 class = _detail_array_view::enable_if_random_it_t<IT>> // FIXME: random iterator is not good enough
																	// (e.g. deque), we
	constexpr ArrayView( IT start, IT end ) noexcept                // need c++17 contiguous iterator
		: _data( &start[0] )
		, _size( end - start )
	{
	}

	/**
	 * construction from a c-style array
	 */
	template<size_t N>
	constexpr ArrayView( T ( &other )[N] ) noexcept
		: _data( other )
		, _size( N )
	{
	}

	/**
	 * construction from a container, in case T is not const (using SFINAE for disambiguation with the other
	 * constructor).
	 */
	template<
		class U,
		class = std::enable_if_t<!std::is_const_v<T> && _detail_array_view::is_compatible_container_v<U, value_type>>>
	constexpr ArrayView( U& other ) noexcept
		: _data( other.data() )
		, _size( other.size() )
	{
	}

	/**
	 * construction from a container, in case T is const (using SFINAE for disambiguation with the other constructor).
	 */
	template<
		class U,
		class = std::enable_if_t<std::is_const_v<T> && _detail_array_view::is_compatible_container_v<U, value_type>>>
	constexpr ArrayView( const U& other ) noexcept
		: _data( other.data() )
		, _size( other.size() )
	{
	}

	/* #### Special member functions #### */
	constexpr ArrayView( const ArrayView& other ) noexcept = default;
	constexpr ArrayView& operator=( const ArrayView& other ) noexcept = default;

	// conversion to view that can't be used to change the underlying data
	operator ArrayView<const T>() const noexcept { return {_data, _size}; }

	/* #### view functions #### */
	constexpr size_type length() const noexcept { return this->size(); }
	constexpr bool      empty() const noexcept { return this->size() == 0; }

	constexpr size_type size_inBytes() const noexcept { return _size * sizeof( value_type ); }

	// clang-format off
	ArrayView<matching_byte_t> asBytes()           noexcept { return { reinterpret_cast<matching_byte_t*>( _data ), size_inBytes() }; }
	ArrayView<const ByteType> asConstBytes() const noexcept { return { reinterpret_cast<const ByteType*> ( _data ), size_inBytes() }; }
	ArrayView<const ByteType> asBytes()      const noexcept { return asConstBytes(); }

	const char*	asConstCharPtr() const noexcept { return reinterpret_cast<const char*>( _data ); }
	const char*	asCharPtr()      const noexcept { return asConstCharPtr(); }

	matching_char_t* asCharPtr() noexcept { return reinterpret_cast<matching_char_t*>( _data ); }

	// clang-format on

	constexpr ArrayView<T> max_subview( size_t offset, size_t count ) const noexcept
	{
		return offset > _size ? ArrayView<T>{} : ArrayView<T>{_data + offset, std::min( count, _size - offset )};
	}

	constexpr ArrayView<T> subview( size_t offset, size_t count ) const
	{
		_throwIfInvalidSubview( offset, count );
		return ArrayView<T>{_data + offset, count};
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
	constexpr ArrayView<T> subview( size_t offset ) const
	{
		_throwIfInvalidSubview( offset, _size - offset );
		return ArrayView<T>{_data + offset, _size - offset};
	}

	constexpr ArrayView<T> max_subview( size_t offset ) const noexcept
	{
		return offset > _size ? ArrayView{} : ArrayView{_data + offset, _size - offset};
	}

	constexpr std::pair<ArrayView<T>, ArrayView<T>> split( size_t offset ) const
	{
		_throwIfOffsetOutOfRange( offset );
		return std::pair<ArrayView<T>, ArrayView<T>>{ArrayView<T>{_data, offset},
													 ArrayView<T>{_data + offset, _size - offset}};
	}

	constexpr std::pair<ArrayView<T>, ArrayView<T>> split( const_iterator splitpoint ) const
	{
		return split( &*splitpoint - _data );
	}

	constexpr bool isValid() const noexcept { return _data != nullptr; }

protected:
	constexpr bool _throwIfOffsetOutOfRange( size_t idx ) const
	{
		return idx <= _size ? true
#ifndef NDEBUG
							: throw std::out_of_range( "Tried to specify offset " + std::to_string( idx )
													   + "into an Array view of size" + std::to_string( _size ) );
#else
							: throw std::out_of_range( "Tried to specify offset that exceeds size of array_view" );
#endif
	}
	constexpr bool _throwIfInvalidSubview( size_t offset, size_t count ) const
	{
		// this will trigger a compile time error, when called in a constexpr context
		// XXX: nasty hack:
		// c++11 constexpr functions need to have a return type other than void, although we are not interested in it
		// here
		return offset + count <= _size
				   ? true
#ifndef NDEBUG
				   : throw std::out_of_range(
					   std::string( "Tried to create a subview that would exceed the original array view." )
					   + "Original size: " + std::to_string( _size ) + ". Offset/Count:" + std::to_string( offset )
					   + "/" + std::to_string( count ) + "\n" );
#else
				   : throw std::out_of_range( "Tried to create a subview that would exceed the original array view." );
#endif
	}

	pointer   _data = nullptr;
	size_type _size = 0;

	// interface for ArrayViewAdaptor
	constexpr pointer       _arrayView_data() noexcept { return _data; }
	constexpr const_pointer _arrayView_data() const noexcept { return _data; }
	constexpr size_type     _arrayView_size() const noexcept { return _size; }
	friend class ArrayViewAdaptor<T, ArrayView<T>>;
};

template<class C, class = typename C::value_type>
constexpr auto view_elements( const C& c ) noexcept -> mart::ArrayView<std::remove_reference_t<decltype( *c.data() )>>
{
	return {c};
}

template<class C, class = typename C::value_type>
constexpr auto view_elements_mutable( C& c ) noexcept
	-> mart::ArrayView<std::remove_const_t<std::remove_reference_t<decltype( *c.data() )>>>
{
	return {c};
}

template<class T>
ConstMemoryView view_bytes( const T& e ) noexcept
{
	return ConstMemoryView( reinterpret_cast<const ByteType*>( &e ), sizeof( e ) );
}

// This is actually the same as "view_bytes", but might be usefull, when you have to make it clear in code
template<class T>
ConstMemoryView view_bytes_const( const T& e ) noexcept
{
	return view_bytes<T>( e );
}

template<class T>
MemoryView view_bytes_mutable( T& e ) noexcept
{
	return MemoryView( reinterpret_cast<ByteType*>( &e ), sizeof( e ) );
}

template<class T>
[[deprecated( "Use view_bytes instead" )]] ConstMemoryView asBytes( const T& e )
{
	return view_bytes( e );
}

template<class T>
[[deprecated( "Use view_bytes_mutable instead" )]] MemoryView asMutableBytes( T& e )
{
	return view_bytes_mutable( e );
}

template<class T>
[[deprecated( "Use view_bytes_const instead" )]] ConstMemoryView asConstBytes( const T& e )
{
	return view_bytes_const( e );
}

template<class T>
[[deprecated( "Use view_bytes_mutable instead" )]] MemoryView viewMemory( T& e )
{
	return view_bytes_mutable( e );
}

template<class T>
[[deprecated( "Please use view_bytes instead" )]] ConstMemoryView viewMemory( const T& e )
{
	return view_bytes( e );
}

template<class T>
[[deprecated( "Please use view_bytes instead" )]] ConstMemoryView viewMemoryConst( const T& e )
{
	return view_bytes( e );
}

template<class T>
auto copy_some( ArrayView<T> src, ArrayView<std::remove_const_t<T>> dest ) noexcept -> ArrayView<std::remove_const_t<T>>
{
	auto cnt = std::min( src.size(), dest.size() );
	std::copy_n( src.cbegin(), cnt, dest.begin() );
	return dest.subview( cnt );
}

template<class T>
auto copy( ArrayView<T> src, ArrayView<std::remove_const_t<T>> dest ) -> ArrayView<std::remove_const_t<T>>
{
	assert( src.size() <= dest.size() );
	if( src.size() > dest.size() ) { return ArrayView<std::remove_const_t<T>>{}; }
	std::copy_n( src.cbegin(), src.size(), dest.begin() );
	return dest.subview( src.size() );
}

} // namespace mart

#endif
