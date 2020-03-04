#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_REF_CNT_PTR_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_REF_CNT_PTR_H
/**
 * RefCntPtr.h (mart-common/experimetnal)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  Provides an (specialized) implementation of a ref counting pointer
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <type_traits>
#include <utility>

/* Proprietary Library Includes */
/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace experimental {

namespace detail {
namespace RcPtr {
template<typename U, typename = int>
struct Has_dec : std::false_type {
};

template<typename U>
struct Has_dec<U, decltype( (void)std::declval<U>().rcInc(), 0 )> : std::true_type {
};

template<typename U, typename = int>
struct Has_inc : std::false_type {
};

template<typename U>
struct Has_inc<U, decltype( (void)std::declval<U>().rcDec(), 0 )> : std::true_type {
};

template<typename U, typename = int>
struct Has_content : std::false_type {
};

template<typename U>
struct Has_content<U, decltype( (void)std::declval<U>().rcContent(), 0 )> : std::true_type {
};

} // namespace RcPtr
} // namespace detail

/**
 * This class is meant to be a building block for clases that use intrusive ref counting
 * It is only concerned with correctly incrementing and decrementing ref counts, but doesn't
 * assign special meaning to them (e.g. it doesn't call delete when the refcount goes to zero)
 */
template<class E, class T = typename std::decay<decltype( *std::declval<E>().rcContent() )>::type>
class RcPtr {
	E* _ptr = nullptr;

public:
	// check presence of required member functions
	static_assert( detail::RcPtr::Has_dec<E>::value && detail::RcPtr::Has_dec<E>::value
					   && detail::RcPtr::Has_content<E>::value,
				   "Type must provide rcInc() and rcDec() and rcContent()" );

	// The refcount is stored as part of the target object.
	// As this might be a wrapper around the actual type we want to point to,
	// the user has the ability to treat the pointer as a class refering to a different type
	// using T = typename std::decay<decltype(*std::declval<E>().rcContent()) > ::type;

	// ctors
	RcPtr() = default;
	RcPtr( E& e )
		: RcPtr( &e )
	{
	}
	RcPtr( E* e )
		: _ptr{e}
	{
		_condInc();
	}

	// special member functions
	RcPtr( const RcPtr& other )
		: _ptr( other._ptr )
	{
		_condInc();
	}
	RcPtr( RcPtr&& other )
		: _ptr{std::exchange( other._ptr, nullptr )}
	{
	}

	RcPtr& operator=( const RcPtr& other )
	{
		// note: increment before decrement to ensure sane behavior in case of self assignment
		other._condInc();
		this->_condDec();
		this->_ptr = other._ptr;
		return *this;
	}

	RcPtr& operator=( RcPtr&& other )
	{
		this->_condDec();
		this->_ptr = std::exchange( other._ptr, nullptr );
		return *this;
	}
	void reset( E* other = nullptr ) { *this = RcPtr<E>( other ); }
	~RcPtr() { _condDec(); }

	// clang-format off
	// pointer operations
	T& operator*()	const	{ return *_ptr->rcContent(); }
	T* operator->()	const	{ return _ptr->rcContent(); }
	T* get()		const	{ return _ptr->rcContent(); }

	//check against nullptr
	explicit operator bool() const { return _ptr != nullptr; }
	template<class TE>	friend bool operator==(const RcPtr<E>& l, std::nullptr_t) { return l._ptr == nullptr; }
	template<class TE>	friend bool operator!=(const RcPtr<E>& l, std::nullptr_t) { return l._ptr != nullptr; }
	template<class TE>	friend bool operator==(std::nullptr_t, const RcPtr<E>& l) { return l._ptr == nullptr; }
	template<class TE>	friend bool operator!=(std::nullptr_t, const RcPtr<E>& l) { return l._ptr != nullptr; }
	// clang-format on

private:
	void _condInc() const
	{
		if( _ptr ) { _ptr->rcInc(); }
	}
	void _condDec() const
	{
		if( _ptr ) { _ptr->rcDec(); }
	}
};

} // namespace experimental
} // namespace mart

#endif
