#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_COPYABLE_ATOMIC_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_COPYABLE_ATOMIC_H
/**
 * CopyableAtomic.h (mart-common/experimental)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  Wrappper around std::atomic that provides default copy operations
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <atomic>

/* Proprietary Library Includes */

/* Project Includes */

namespace mart {

/**
 * Drop in replacement for std::atomic that provides a copy constructor and copy assignment operator.
 *
 * Contrary to normal atomics, these atomics don't prevent the generation of
 * default constructor and copy operators for classes they are members of.
 *
 * Copying those atomics is thread safe, but be aware that
 * it doesn't provide any form of synchronization.
 * Their main purpose is to easily enable classes that have those atomics as members to
 * be copied around during setup, while for actually synchronization the normal member functions are used.
 */
template<class T>
class CopyableAtomic : public std::atomic<T> {
public:
	static_assert( sizeof( T ) < 8 && std::is_pod<T>::value, "Class is only meant for trivial buildin types" );

	// defaultinitializes value (std::atomic doesn't)
	constexpr CopyableAtomic() noexcept
		: std::atomic<T>( T{} )
	{
	}

	constexpr CopyableAtomic( T desired ) noexcept
		: std::atomic<T>( desired )
	{
	}

	constexpr CopyableAtomic( const CopyableAtomic<T>& other ) noexcept
		: CopyableAtomic( other.load( std::memory_order_relaxed ) )
	{
	}

	CopyableAtomic& operator=( const CopyableAtomic<T>& other ) noexcept
	{
		this->store( other.load( std::memory_order_relaxed ), std::memory_order_relaxed );
		return *this;
	}

	constexpr CopyableAtomic( CopyableAtomic<T>&& other ) noexcept
		: CopyableAtomic( other.load( std::memory_order_relaxed ) )
	{
	}

	CopyableAtomic& operator=( CopyableAtomic<T>&& other ) noexcept
	{
		this->store( other.load( std::memory_order_relaxed ), std::memory_order_relaxed );
		return *this;
	}
};

} // namespace mart

#endif
