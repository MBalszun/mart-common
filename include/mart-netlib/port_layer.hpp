#ifndef LIB_MART_COMMON_GUARD_NW_PORT_LAYER_H
#define LIB_MART_COMMON_GUARD_NW_PORT_LAYER_H
/**
 * port_layer.h (mart-common/nw)
 *
 * Copyright (C) 2015-2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  Contains all network related functions that need platform specific implementation
 *
 */

#include "basic_types.hpp"

#include <chrono>

namespace mart {
namespace nw {
namespace socks {

namespace port_layer {

using socket_timeout_t = std::uint32_t;
// Define aliases for platform specific types and values
#ifdef MBA_UTILS_USE_WINSOCKS
using native_handle_t = std::uintptr_t;
enum class handle_t : native_handle_t { Invalid = ~0 };
constexpr int socket_error_value = -1;
#else
using native_handle_t = int;
enum class handle_t : native_handle_t { Invalid = -1 };
constexpr int socket_error_value = -1;

#endif // MBA_UTILS_USE_WINSOCKS


inline native_handle_t to_native( handle_t h ) noexcept
{
	return static_cast<native_handle_t>( h );
}

int to_native( Domain domain );
int to_native( TransportType transport_Type );
int to_native( Protocol protocol );

// Wrapper functions for socket related functions, that are specific to a certain platform
bool set_blocking( handle_t socket, bool should_block ) noexcept;

int  close_socket( handle_t handle ) noexcept;
int  getLastSocketError() noexcept;
bool waInit() noexcept;

// handle_t socket( int af, int type, int protocol );
handle_t socket( Domain domain, TransportType transport_type, Protocol protocol = Protocol::Default );

handle_t accept( handle_t sockfd, Sockaddr& addr ) noexcept;
handle_t accept( handle_t sockfd ) noexcept;

int connect( handle_t handle, const Sockaddr& addr ) noexcept;
int bind( handle_t s, const Sockaddr& addr ) noexcept;
int listen( handle_t sockfd, int backlog ) noexcept;

int setsockopt( handle_t handle, SocketOptionLevel level, SocketOption optname, byte_range data ) noexcept;
int getsockopt( handle_t handle, SocketOptionLevel level, SocketOption optname, byte_range_mut& buffer ) noexcept;

txrx_size_t send( handle_t handle, byte_range buf, int flags ) noexcept;
txrx_size_t sendto( handle_t handle, byte_range buf, int flags, const Sockaddr& to ) noexcept;

txrx_size_t recv( handle_t handle, byte_range_mut buf, int flags ) noexcept;
txrx_size_t recvfrom( handle_t handle, byte_range_mut buf, int flags, Sockaddr& from ) noexcept;

bool set_timeout( handle_t handle, Direction direction, std::chrono::microseconds timeout ) noexcept;
std::chrono::microseconds get_timeout( handle_t handle, Direction direction ) noexcept;


struct sockaddr_in : mart::nw::socks::Sockaddr {
	sockaddr_in()
		: sockaddr_in( Storage {} )
	{
	}
	sockaddr_in( const sockaddr_in& other )
		: sockaddr_in( other._storage )
	{
	}

	sockaddr_in& operator=( const sockaddr_in& other ) noexcept
	{
		_storage = other._storage;
		return *this;
	}

	explicit sockaddr_in( const ::sockaddr_in& native ) noexcept;
	sockaddr_in( mart::nw::uint32_net_t address, mart::nw::uint16_net_t port ) noexcept;

	const ::sockaddr_in& native() const noexcept;
	::sockaddr_in&       native() noexcept;

	mart::nw::uint32_net_t address() const noexcept;
	mart::nw::uint16_net_t port() const noexcept;

private:
	// this should have at least the same size and alignment as the platform's sockaddr_in
	struct alignas( 4 ) Storage {
		char raw_bytes[16];
	};
	Storage _storage {};

	// All constructors forward to this
	explicit sockaddr_in( const sockaddr_in::Storage& src )
		: Sockaddr( mart::nw::socks::Domain::Inet, byte_range_from_pod( _storage ) )
		, _storage( src )
	{
	}
};

struct sockaddr_in6 : mart::nw::socks::Sockaddr {
	sockaddr_in6()
		: sockaddr_in6( Storage {} )
	{
	}
	sockaddr_in6( const sockaddr_in6& other )
		: sockaddr_in6( other._storage )
	{
	}

	sockaddr_in6& operator=( const sockaddr_in6& other )
	{
		_storage = other._storage;
		return *this;
	}

	explicit sockaddr_in6( const ::sockaddr_in6& native );

private:
	// this should have the same size and alignment as the platform's sockaddr_in
	struct alignas( 8 ) Storage {
		char raw_bytes[32];
	};
	Storage _storage {};

	explicit sockaddr_in6( const sockaddr_in6::Storage& src )
		: Sockaddr( mart::nw::socks::Domain::Inet6, byte_range_from_pod( _storage ) )
		, _storage( src )
	{
	}
};
const char* inet_net_to_pres( mart::nw::socks::Domain af, const void* src, char* dst, size_t size );
int         inet_pres_to_net( mart::nw::socks::Domain af, const char* src, void* dst );
} // namespace port_layer
} // namespace socks


} // namespace nw
} // namespace mart

#endif
