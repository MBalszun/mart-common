#ifndef LIB_MART_COMMON_GUARD_CIRCULAR_BUFFER_H
#define LIB_MART_COMMON_GUARD_CIRCULAR_BUFFER_H
/**
 * circular_buffer.h (mart-common/experimental)
 *
 * Copyright (C) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides a simple circular bufferimplemenation
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <cassert>
#include <type_traits>

/* Proprietary Library Includes */

/* Project Includes */

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

namespace detail_circular_buffer {
using index_base_type_t = unsigned int;

constexpr index_base_type_t max_multiple_that_fits_into_base( index_base_type_t n ) noexcept
{
	// Half of the natural modulo of index_base_type_t
	constexpr auto half_max = index_base_type_t( 1 ) << (sizeof( index_base_type_t ) * 8 - 1);
	const auto     times    = half_max / n;
	return half_max + times * n;
}

template<index_base_type_t Limit, bool IsPowerOfTwo = false>
struct IdxWrapper {
	static constexpr auto mod = max_multiple_that_fits_into_base( Limit );

	enum class idx_t : index_base_type_t {};

	friend constexpr index_base_type_t to_integral( idx_t i ) noexcept { return static_cast<unsigned int>( i ); }

	friend constexpr idx_t operator++( idx_t& idx, int ) noexcept
	{
		auto val = to_integral( idx );
		if( val < mod - 1u ) {
			idx = idx_t {val + 1};
		} else {
			idx = idx_t {};
		}
		return idx_t {val};
	}

	friend constexpr idx_t operator+( idx_t l, index_base_type_t r ) noexcept
	{
		return idx_t {to_integral(l) + r};
	};

	friend constexpr index_base_type_t operator-( idx_t l, idx_t r ) noexcept
	{
		assert( l >= r );
		return to_integral( l ) - to_integral( r );
	};


	friend constexpr index_base_type_t operator%( idx_t i, index_base_type_t b ) noexcept
	{
		return to_integral( i ) % b;
	};
};

template<index_base_type_t Limit>
struct IdxWrapper<Limit, true> {
	using idx_t = index_base_type_t;
	friend constexpr index_base_type_t       to_integral( idx_t i ) noexcept { return i; }
};

constexpr bool n_is_power_of_two( index_base_type_t N ) noexcept
{
	return ( (N & ( N - 1 )) == 0 );
}

template<index_base_type_t Limit>
using idx_t = typename IdxWrapper<Limit, n_is_power_of_two( Limit )>::idx_t;

} // namespace detail_circular_buffer

template<class T, unsigned int N>
class circular_buffer {
	static_assert( std::is_trivially_destructible_v<T>,
				   "This circular buffer implementation currently only supports trivially destructible types" );

	// This will either be
	// - if N is a power of two: a plain unsigned int (which has all the properties we want),
	// - otherwise: an enumerator with mathematical operators defined in such a way that it
	//    behaves like an unsigned type with a modulo that is a (large) multiple of N
	using index_type = detail_circular_buffer::idx_t<N>;

	T data[N] {};

	index_type m_next_read {};
	index_type m_next_write {};

	static constexpr unsigned int map_to_array_pos( index_type i ) noexcept { return i % N; }

public:
	constexpr circular_buffer() noexcept = default;
	void push_back( const T& value )
	{
		assert( this->size() < N );
		data[map_to_array_pos( m_next_write++ )] = value;
	}

	T pop_front() noexcept
	{
		assert( this->size() > 0 );
		return std::move( data[map_to_array_pos( m_next_read++ )] );
	}

	void pop_front( T& out ) noexcept
	{
		assert( this->size() > 0 );
		out = std::move( data[map_to_array_pos( m_next_read++ )] );
	}

	constexpr auto operator[]( int i ) { return data[map_to_array_pos( m_next_read + i )]; }
	constexpr auto operator[]( int i ) const { return data[map_to_array_pos( m_next_read + i )]; }

	constexpr int size() const { return m_next_write - m_next_read; }

	constexpr bool empty() const { return size() == 0; }
};

} // namespace mart

#endif // !LIB_MART_COMMON_GUARD_CIRCULAR_BUFFER_H
