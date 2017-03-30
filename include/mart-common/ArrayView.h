#ifndef LIB_MART_COMMON_GUARD_ARRAY_VIEW_H
#define LIB_MART_COMMON_GUARD_ARRAY_VIEW_H
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator> //iterator tags
#include <stdexcept>
#include <string> //exception messages

/* Proprietary Library Includes */
#include "./cpp_std/type_traits.h"

/* Project Includes */
#include "ArrayViewAdaptor.h"

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

template <class T>
class ArrayView;

using ByteType = uint8_t;

using MemoryView	  = ArrayView<ByteType>;
using ConstMemoryView = ArrayView<const ByteType>;

template <class T>
class ArrayView : public ArrayViewAdaptor<T, ArrayView<T>> {
public:
	using value_type	= typename ArrayViewAdaptor<T, ArrayView<T>>::value_type;
	using size_type		= typename ArrayViewAdaptor<T, ArrayView<T>>::size_type;
	using pointer		= typename ArrayViewAdaptor<T, ArrayView<T>>::pointer;
	using const_pointer = typename ArrayViewAdaptor<T, ArrayView<T>>::const_pointer;
	using iterator		= typename ArrayViewAdaptor<T, ArrayView<T>>::iterator;
	using const_iterator = typename ArrayViewAdaptor<T, ArrayView<T>>::const_iterator;

private:
	/* ### Type traits short hands ### */
	template <class IT>
	struct is_random_it {
		static constexpr bool value = std::is_same<typename std::iterator_traits<IT>::iterator_category,
												   std::random_access_iterator_tag>::value;
	};

	template <class IT>
	using enable_if_random_it_t = mart::enable_if_t<is_random_it<IT>::value>;

	template <class U>
	static constexpr auto is_compatible_container_helper( std::nullptr_t )
		-> decltype( ( std::declval<U>().data() + std::declval<U>().size() ) == std::declval<U>().data() )
	{
		return
			std::is_same<
				typename std::iterator_traits<typename U::iterator>::iterator_category,
				std::random_access_iterator_tag
			>::value
		 && (	 std::is_same<typename U::value_type, mart::remove_const_t<value_type>>::value
			  || std::is_same<typename U::value_type,						value_type>::value
			);
	};

	template <class U>
	static constexpr auto is_compatible_container_helper( U* ) -> bool
	{
		return false;
	};

	template <class U>
	struct is_compatible_container {
		/* IMPLEMENTATION NOTE:
		 * There are two overloads of is_compatible_container_helper:
		 * one taking an argument of type nullptr_t
		 * one taking an argument of type U*
		 *
		 * If U doesn't have the member functions data() and size(), the first overload SFINAEs away,
		 * leaving only the second one which always returns false
		 *
		 * If U has both member functions the first one winns as nullptr_t is an exact match, whereas U* needs a conversion.
		 * The first one then checks if the container has the correct value type
		 *
		 */
		static constexpr bool value = is_compatible_container_helper<U>( nullptr );
	};

	template <class U, class K>
	struct transfer_constness {
		using type = mart::conditional_t<std::is_const<U>::value, mart::add_const_t<K>, K>;
	};

	template <class U, class K>
	using transfer_constness_t = typename transfer_constness<U, K>::type;

	//const ByteType if T is const, ByteType otherwise
	using matching_byte_t = transfer_constness_t<T, ByteType>;

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
	template <class IT, class = enable_if_random_it_t<IT>> // FIXME: random iterator is not good enough (e.g. deque), we
	constexpr ArrayView(IT start, IT end) noexcept		   // need c++17 contiguous iterator
		: _data( &start[0] )
		, _size( end - start )
	{
	}

	/**
	 * construction from a c-style array
	 */
	template <size_t N>
	constexpr ArrayView( T ( &other )[N] ) noexcept
		: _data( other )
		, _size( N )
	{
	}

	/**
	 * construction from a container, in case T is not const (using SFINAE for disambiguation with the other
	 * constructor).
	 */
	template <class U, class = mart::enable_if_t<!std::is_const<T>::value && is_compatible_container<U>::value>>
	constexpr ArrayView( U& other ) noexcept
		: _data( other.data() )
		, _size( other.size() )
	{
	}

	/**
	 * construction from a container, in case T is const (using SFINAE for disambiguation with the other constructor).
	 */
	template <class U, class = mart::enable_if_t<std::is_const<T>::value && is_compatible_container<U>::value>>
	constexpr ArrayView( const U& other ) noexcept
		: _data( other.data() )
		, _size( other.size() )
	{
	}

	/* #### Special member functions #### */
	constexpr ArrayView( const ArrayView<T>& other ) = default;
	ArrayView<T>& operator=( const ArrayView<T>& other ) = default;

	// conversion to view that can't be used to change the underlying data
	operator ArrayView<const T>() const noexcept { return {_data, _size}; }

	/* #### view functions #### */
	constexpr size_type length() const noexcept { return this->size(); }
	constexpr bool		empty()  const noexcept { return this->size() == 0; }

	constexpr size_type size_inBytes() const noexcept { return _size * sizeof( value_type ); }

	// clang-format off
			  ArrayView<matching_byte_t> asBytes()			  noexcept { return { reinterpret_cast<matching_byte_t*>( _data ), size_inBytes() }; }
	constexpr ArrayView<const ByteType> asConstBytes()	const noexcept { return { reinterpret_cast<const ByteType*> ( _data ), size_inBytes() }; }
	constexpr ArrayView<const ByteType> asBytes()		const noexcept { return asConstBytes(); }

	constexpr const char*			asConstCharPtr() const { return reinterpret_cast<const char*>( _data ); }
	constexpr const char*			asCharPtr()		 const { return asConstCharPtr(); }
	transfer_constness_t<T, char>*	asCharPtr()			   { return reinterpret_cast<transfer_constness_t<T, char>*>( _data ); }

	// clang-format on

	constexpr ArrayView<T> max_subview( size_t offset, size_t count ) const
	{
		return offset > _size ? ArrayView<T>{} : ArrayView<T>{_data + offset, std::min( count, _size - offset )};
	}

	constexpr ArrayView<T> subview( size_t offset, size_t count ) const
	{
		return _throwIfInvalidSubview( offset, count ), // found a usage for the comma (,) operator ;)
			   ArrayView<T>{_data + offset, count};
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
		return _throwIfInvalidSubview( offset, _size - offset ), // found a usage for the comma (,) operator ;)
			   ArrayView<T>{_data + offset, _size - offset};
	}

	constexpr ArrayView<T> max_subview( size_t offset ) const
	{
		return offset > _size ? ArrayView{} : ArrayView{_data + offset, _size - offset};
	}

	constexpr std::pair<ArrayView<T>, ArrayView<T>> split(size_t offset) const
	{
		return _throwIfOffsetOutOfRange(offset), std::pair<ArrayView<T>, ArrayView<T>>{ ArrayView<T>{_data,offset}, ArrayView<T>{_data + offset,_size - offset} };
	}

	constexpr std::pair<ArrayView<T>, ArrayView<T>> split(const_iterator splitpoint) const
	{
		return split(&*splitpoint - _data);
	}

	constexpr bool isValid() const noexcept { return _data != nullptr; }

protected:
	constexpr bool _throwIfOutOfRange( size_t idx ) const
	{
		return idx < _size ? true : throw std::out_of_range( "Tried to access " + std::to_string( idx )
															 + "th element of an Array view of size"
															 + std::to_string( _size ) );
	}
	constexpr bool _throwIfOffsetOutOfRange(size_t idx) const
	{
		return idx <= _size ? true : throw std::out_of_range("Tried to specify offset " + std::to_string(idx)
															+ "into an Array view of size"
															+ std::to_string(_size));
	}
	constexpr bool _throwIfInvalidSubview( size_t offset, size_t count ) const
	{
		// this will trigger a compile time error, when called in a constexpr context
		// XXX: nasty hack:
		// c++11 constexpr functions need to have a return type other than void, although we are not interested in it
		// here
		return offset + count <= _size
				   ? true
				   : throw std::out_of_range(
						 std::string( "Tried to create a subview that would exceed the original array view." )
						 + "Original size: "
						 + std::to_string( _size )
						 + ". Offset/Count:"
						 + std::to_string( offset )
						 + "/"
						 + std::to_string( count )
						 + "\n" );
	}

	pointer   _data = nullptr;
	size_type _size = 0;

	// interface for ArrayViewAdaptor
	pointer					_arrayView_data()		noexcept { return _data; }
	constexpr const_pointer _arrayView_data() const noexcept { return _data; }
	constexpr size_type		_arrayView_size() const noexcept { return _size; }
	friend class ArrayViewAdaptor<T, ArrayView<T>>;
};

template <class T>
/*c++14: [[deprecated]] */ constexpr MemoryView viewMemory( T& e )
{
	return MemoryView( reinterpret_cast<ByteType*>( &e ), sizeof( e ) );
}

template <class T>
/*c++14: [[deprecated]] */ constexpr ConstMemoryView viewMemory( const T& e )
{
	return ConstMemoryView( reinterpret_cast<const ByteType*>( &e ), sizeof( e ) );
}

template <class T>
/*c++14: [[deprecated]] */ constexpr ConstMemoryView viewMemoryConst( const T& e )
{
	return ConstMemoryView( reinterpret_cast<const ByteType*>( &e ), sizeof( e ) );
}

template <class T>
constexpr MemoryView asBytes( T& e )
{
	return MemoryView( reinterpret_cast<ByteType*>( &e ), sizeof( e ) );
}

template <class T>
constexpr ConstMemoryView asBytes( const T& e )
{
	return ConstMemoryView( reinterpret_cast<const ByteType*>( &e ), sizeof( e ) );
}

template <class T>
constexpr ConstMemoryView asConstBytes( const T& e )
{
	return ConstMemoryView( reinterpret_cast<const ByteType*>( &e ), sizeof( e ) );
}

template <class T>
auto copy_some( ArrayView<T> src, ArrayView<mart::remove_const_t<T>> dest ) -> ArrayView<mart::remove_const_t<T>>
{
	auto cnt = std::min( src.size(), dest.size() );
	std::copy_n( src.cbegin(), cnt, dest.begin() );
	return dest.subview( cnt );
}

template <class T>
auto copy( ArrayView<T> src, ArrayView<mart::remove_const_t<T>> dest ) -> ArrayView<mart::remove_const_t<T>>
{
	if( src.size() > dest.size() ) {
		assert( false );
		return ArrayView<mart::remove_const_t<T>>{};
	}
	std::copy_n(src.cbegin(), src.size() , dest.begin());
	return dest.subview( src.size() );
}

template <class T>
bool equal( ArrayView<T> l, ArrayView<T> r )
{
	if( l.size() != r.size() ) {
		return false;
	}
	return std::equal( l.begin(), l.end(), r.begin() );
}

} // namespace mart

#endif