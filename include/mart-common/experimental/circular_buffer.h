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
struct circular_buffer {
	static_assert( std::is_trivially_destructible_v<T>,
				   "This circular buffer implementation currently only supports trivially destructible types" );

	std::array<T, (std::size_t)N> data;
	using index_type = int;
	index_type next_read{};
	index_type next_write{};

	static index_type next( index_type current ) { return ( current + 1 ) % N; }

	void push_back( const T& value )
	{
		data[(std::size_t)next_write] = value;
		next_write					  = next( next_write );
	};

	T pop_front()
	{
		auto ti   = next_read;
		next_read = next( next_read );
		return data[(std::size_t)ti];
	};

	void pop_front( T& out )
	{
		out		  = data[(std::size_t)next_read];
		next_read = next( next_read );
	};

	int size() const {
		if( next_write >= next_read )
			return next_write - next_read;
		else
			return next_write + N - next_read;
	}
};

} // namespace mart

#endif // !LIB_MART_COMMON_GUARD_CIRCULAR_BUFFER_H
