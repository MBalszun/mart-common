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
#include <array>
#include <type_traits>

/* Proprietary Library Includes */

/* Project Includes */

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

template<class T, int N>
class circular_buffer {
	static_assert( std::is_trivially_destructible_v<T>,
				   "This circular buffer implementation currently only supports trivially destructible types" );
	using index_type = int;

	std::array<T, (std::size_t)N> data{};

	index_type m_next_read{};
	index_type m_next_write{};

	static index_type next( index_type current ) { return ( current + 1 ) % N; }

public:
	constexpr circular_buffer() = default;

	constexpr void push_back( const T& value )
	{
		data[(std::size_t)m_next_write] = value;
		m_next_write                    = next( m_next_write );
	};

	constexpr T pop_front()
	{
		const auto ti = m_next_read;
		m_next_read   = next( m_next_read );
		return std::move( data[(std::size_t)ti] );
	};

	constexpr void pop_front( T& out )
	{
		out         = data[(std::size_t)m_next_read];
		m_next_read = next( m_next_read );
	};

	constexpr auto operator[]( int i ) { return data[( m_next_read + i ) % N]; }
	constexpr auto operator[]( int i ) const { return data[( m_next_read + i ) % N]; }

	constexpr int size() const
	{
		if( m_next_write >= m_next_read )
			return m_next_write - m_next_read;
		else
			return m_next_write + N - m_next_read;
	}

	constexpr bool empty() const { return size() == 0; }
};

} // namespace mart

#endif // !LIB_MART_COMMON_GUARD_CIRCULAR_BUFFER_H
