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

// detect windows os. TODO: use other guards if necessary
// detect windows os. TODO: use other guards if necessary
#ifdef _MSC_VER
#define MBA_UTILS_USE_WINSOCKS

// assume little endian for windows
#define MBA_ORDER_LITTLE_ENDIAN 1
#define MBA_BYTE_ORDER MBA_ORDER_LITTLE_ENDIAN
#else
// use gcc primitives
#define MBA_ORDER_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define MBA_BYTE_ORDER __BYTE_ORDER__
#endif

#include <cstddef>
#include <cstdint>

/* ######## WINDOWS ######### */

// Including Windows.h (indirectly) tends to import some nasty macros. in particular min and max

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

struct sockaddr;
struct sockaddr_in;
struct sockaddr_un;
struct sockaddr_in6;

namespace mart {
namespace nw {
namespace socks {
namespace port_layer {

// Define aliases for platform specific types and values
#ifdef MBA_UTILS_USE_WINSOCKS
using handle_t                        = std::uintptr_t;
using address_len_t                   = int;
using buf_size_t                      = int;
using txrx_size_t                     = int;
using socket_timeout_t                = std::uint32_t;
constexpr handle_t invalid_handle     = ( handle_t )( ~0 );
constexpr int      socket_error_value = -1;
#else
using handle_t                               = int;
using address_len_t                          = unsigned int;
using txrx_size_t                            = std::ptrdiff_t;
using buf_size_t                             = std::size_t;
using socket_timeout_t                       = std::uint32_t;
static constexpr handle_t invalid_handle     = -1;
static constexpr int      socket_error_value = -1;
#endif // MBA_UTILS_USE_WINSOCKS

struct sockaddr;
struct sockaddr_un;
struct sockaddr_in;
struct sockaddr_in6;

enum class Domain {
	local,
	inet,
	inet6,
};
enum class TransportType {
	stream,
	datagram,
	seqpacket,
};

enum class SocketOptionLevel { sol_socket };

enum class SocketOption { so_rcvtimeo, so_sndtimeo };

int to_native( Domain domain );
int to_native( TransportType transport_Type );

// Wrapper functions for socket related functions, that are specific to a certain platform
bool set_blocking( handle_t socket, bool should_block ) noexcept;

int  close_socket( handle_t handle ) noexcept;
int  getLastSocketError() noexcept;
bool waInit() noexcept;

handle_t socket( int af, int type, int protocol );
handle_t socket( Domain domain, TransportType transport_type );
handle_t accept( handle_t sockfd, sockaddr* addr, address_len_t* addrlen ) noexcept;
int      connect( handle_t handle, const sockaddr* name, address_len_t namelen ) noexcept;
int      bind( handle_t s, const sockaddr* addr, address_len_t namelen ) noexcept;
int      listen( handle_t sockfd, int backlog ) noexcept;

int setsockopt( handle_t handle, int level, int optname, const char* optval, address_len_t optlen ) noexcept;
int getsockopt( handle_t handle, int level, int optname, char* optval, address_len_t* optlen ) noexcept;

int setsockopt(
	handle_t handle, SocketOptionLevel level, SocketOption optname, const char* optval, address_len_t optlen ) noexcept;
int getsockopt(
	handle_t handle, SocketOptionLevel level, SocketOption optname, char* optval, address_len_t* optlen ) noexcept;

txrx_size_t send( handle_t handle, const char* buf, buf_size_t len, int flags ) noexcept;
txrx_size_t	sendto( handle_t handle, const char* buf, buf_size_t len, int flags, const sockaddr* to, address_len_t tolen ) noexcept;
txrx_size_t recv( handle_t handle, char* buf, buf_size_t len, int flags ) noexcept;
txrx_size_t recvfrom( handle_t handle, char* buf, buf_size_t len, int flags, sockaddr* from, address_len_t* fromlen ) noexcept;

} // namespace port_layer
} // namespace socks

namespace ip {
namespace port_layer {

const char* inet_net_to_pres( int af, const void* src, char* dst, size_t size );
int         inet_pres_to_net( int af, const char* src, void* dst );

} // namespace port_layer
} // namespace ip
} // namespace nw
} // namespace mart

#endif
