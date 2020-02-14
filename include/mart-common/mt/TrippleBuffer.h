#ifndef LIB_MART_COMMON_GUARD_MT_TRIPPLE_BUFFER_H
#define LIB_MART_COMMON_GUARD_MT_TRIPPLE_BUFFER_H
/**
 * TrippleBuffer.h (mart-common/mt)
 *
 * Copyright (C) 2018-2020: Michael Balszun <michael.balszun@tum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@tum.de>
 * @brief:	A tripple buffer implementation
 *
 */

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
 * void consumer() {
 * 	for (;;) {
 * 		buffer.fetch_update();
 * 		std::string& t = buffer.get_read_buffer();
 * 		std::cout << t << std::endl;
 * 	}
 * }
 */

/**
 * Threadsafe tripple buffer datastructure
 *
 * Allows to decouple the rate at which the producer
 * generates new values and the consumer consumes them.
 *
 * Does not provide an integrated way to efficiently wait for new content
 *
 */
template<class T>
class TrippleBuffer {
	// The three slots and three indices:
	// - read_idx:   Number of the slot currently read from
	// - write_idx:  Number of the slot currently written to
	// - buffer_idx: Number of the slot that was last updated but yet requested by the reader.

	T data[3]{};

	struct alignas( std::uint32_t ) Index {
		std::uint16_t idx;
		bool          new_data;
	};
	static_assert( sizeof( Index ) <= sizeof( std::uint32_t ) );
	static_assert( std::atomic<Index>::is_always_lock_free );

	Index read_idx{0, false};
	Index write_idx{1, false};

	std::atomic<Index> buffer_idx{Index{2, false}};

public:
	constexpr TrippleBuffer() noexcept( noexcept(T{}) ) = default;
	constexpr explicit TrippleBuffer( const T& init ) noexcept( noexcept( T{init} ) )
		: data{init, init, init}
	{
	}

	T& get_write_buffer() noexcept { return data[write_idx.idx]; }

	// Todo switch this to returning const reference in a future version
	T& get_read_buffer() noexcept { return data[read_idx.idx]; }

	// This is mostly useful, if we want to move out the data from the buffer
	T&       get_mutable_read_buffer() noexcept { return data[read_idx.idx]; }
	const T& get_immutable_read_buffer() const noexcept  { return data[read_idx.idx]; }

	/*
	 * Returns true if new buffer has been commited since last call to fetch_update,
	 * and swaps indices for read and buffer slot.
	 * Otherwise returns false and doesn't change indices
	 */
	bool fetch_update() noexcept
	{
		if( !buffer_idx.load().new_data ) {
			// no new content since last fetch
			return false;
		}
		// mark current slot as outdated so we don't refetch it later
		// and swap with buffer slot
		read_idx.new_data = false;
		read_idx          = buffer_idx.exchange( read_idx );
		return true;
	}

	void commit() noexcept
	{
		// mark current slot as new
		// and swap with buffer slot
		write_idx.new_data = true;
		write_idx          = buffer_idx.exchange( write_idx );
	}
};

} // namespace mt
} // namespace mart

#endif
