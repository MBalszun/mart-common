#ifndef LIB_MART_COMMON_GUARD_MT_TRIPPLE_BUFFER_H
#define LIB_MART_COMMON_GUARD_MT_TRIPPLE_BUFFER_H
/**
 * TrippleBuffer.h (mart-common/mt)
 *
 * Copyright (C) 018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	A tripple buffer implementation
 *
 */

#include <array>
#include <atomic>
#include <cstdint>

namespace mart {
namespace mt {

/*
 * Usage example:
 *
 * TrippleBuffer<std::string> buffer;
 *
 * void producer() {
 * 	for (;;) {
 * 		std::string& t = buffer.get_write_buffer();
 * 		t = "Hello";
 * 		buffer.commit();
 * 	}
 * }
 *
 * void consumber() {
 * 	for (;;) {
 * 		buffer.fetch_update();
 * 		std::string& t = buffer.get_read_buffer();
 * 		std::cout << t << std::endl;
 * 	}
 * }
 */

template<class T>
class TrippleBuffer {
	// The three buffer slots
	// - One which we are currently reading from
	// - One which we are currently writing to
	// - One intermediate buffer
	// Which position in the array fulfills which role changes dynamically
	std::array<T, 3> data{};

	struct alignas( 2 * sizeof( std::uint16_t ) ) Index {
		std::uint16_t idx;
		bool		  new_data;
	};

	Index read_idx{0, false};
	Index write_idx{1, false};

	std::atomic<Index> buffer_idx{Index{2, false}};
	static_assert( std::atomic<Index>::is_always_lock_free );

public:
	T& get_write_buffer() { return data[write_idx.idx]; }
	T& get_read_buffer() { return data[read_idx.idx]; }

	TrippleBuffer() = default;
	explicit TrippleBuffer( const T& init )
		: data{init, init, init}
	{
	}

	bool fetch_update()
	{
		if( !buffer_idx.load().new_data ) {
			// no new content since last fetch
			return false;
		}
		// mark current slot as outdated so we don't refetch it later
		// and swap with buffer slot
		read_idx.new_data = false;
		read_idx		  = buffer_idx.exchange( read_idx );
		return true;
	}

	void commit()
	{
		// mart current slot as new
		// and swap with buffer slot
		write_idx.new_data = true;
		write_idx		   = buffer_idx.exchange( write_idx );
	}
};
}
}

#endif
