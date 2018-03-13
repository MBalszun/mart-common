#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_TRIPPLE_BUFFER_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_TRIPPLE_BUFFER_H
/**
 * TrippleBuffer.h (mart-common/experimental/mt)
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

#include <atomic>
#include <array>
#include <cstdint>

namespace mart {
namespace experimental {
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
template< class T >
class TrippleBuffer {
	// The three buffer slots
	// - One which we are currently reading from
	// - One which we are currently writing to
	// - One intermediate buffer
	// Which position in the array fulfills which role changes dynamically
	std::array< T, 3 > data{};

	// NOTE: Those pointers could be replaced by indexes
	// TODO: Investigate if those could be merged with the
	//		 counter variables
	T*				   read_ptr   = &data[0];
	T*				   write_ptr  = &data[1];
	std::atomic< T* >  buffer_ptr = &data[2];

	// NOTE: We rely on overflowing unsigned integers wrapping around
	std::uint32_t				 read_cnt{ 0 };
	std::atomic< std::uint32_t > write_cnt{ 0 };

public:
	T& get_write_buffer() { return *write_ptr; }
	T& get_read_buffer() { return *read_ptr; }

	TrippleBuffer() = default;
	explicit TrippleBuffer(const T& init)
		: data{init,init,init}
	{
	}

	bool fetch_update()
	{
		//MAINTENENCE NOTE:
		// The logic in this function is such that it works
		// in case of unsigned integer overflow (wrap around)
		// DON'T break that assumption!

		if( read_cnt == write_cnt ) {
			// no new content since last fetch
			return false;
		}
		do {
			read_cnt = write_cnt;
			read_ptr = buffer_ptr.exchange( read_ptr );
			// if the writer just published another update we do the swap again
		} while( read_cnt != write_cnt );
		return true;
	}

	void commit()
	{
		write_ptr = buffer_ptr.exchange( write_ptr );
		write_cnt++;
	}
};

}
}
}

#endif