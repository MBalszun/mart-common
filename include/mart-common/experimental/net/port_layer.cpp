#include "port_layer.hpp"

/**
 * port_layer.cpp (mart-common/nw)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  Contains all network related functions that need platform specific implementation
 *
 */

/* ######## INCLUDES ######### */
#include <cassert>
#include <cstdio>
#include <type_traits>

// Include OS-specific headers
#ifdef MBA_UTILS_USE_WINSOCKS


/* ######## WINDOWS ######### */

// Including Windows.h (indirectly) tends to import some nasty macros. in particular min and max

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <afunix.h>
#pragma comment( lib, "Ws2_32.lib" )

#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h> //close
#endif
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */


namespace mart {
namespace nw {
namespace socks {
namespace port_layer {

// Define aliases for platform specific types and values
#ifdef MBA_UTILS_USE_WINSOCKS

static_assert( std::is_same_v<handle_t, SOCKET> );
static_assert( std::is_same_v<std::remove_const_t<decltype( invalid_handle )>,      decltype( INVALID_SOCKET )> );
static_assert( std::is_same_v<std::remove_const_t<decltype( socket_error_value )>,  decltype( SOCKET_ERROR )> );

static_assert( invalid_handle == INVALID_SOCKET );
static_assert( socket_error_value == SOCKET_ERROR );
#else

static_assert( std::is_same_v<handle_t, int> );
static_assert( std::is_same_v<address_len_t, socklen_t> );
static_assert( std::is_same_v<txrx_size_t, ssize_t> );

static_assert( std::is_same_v<std::remove_const_t<decltype( invalid_handle )>, decltype( -1 )> );
static_assert( invalid_handle == -1 );

static_assert( std::is_same_v<std::remove_const_t<decltype( socket_error_value )>, decltype( -1 )> );
static_assert( socket_error_value == -1 );

#endif // MBA_UTILS_USE_WINSOCKS

// Wrapper functions for socket related functions, that are specific to a certain platform
bool set_blocking( handle_t socket, bool should_block ) noexcept
{
	bool ret = true;
#ifdef MBA_UTILS_USE_WINSOCKS
	// from
	// http://stackoverflow.com/questions/5489562/in-win32-is-there-a-way-to-test-if-a-socket-is-non-blocking/33087879
	/// @note windows sockets are created in blocking mode by default
	// currently on windows, there is no easy way to obtain the socket's current blocking mode since WSAIsBlocking was
	// deprecated
	u_long non_blocking = should_block ? 0 : 1;
	ret                 = NO_ERROR == ioctlsocket( socket, FIONBIO, &non_blocking );
#else
	const int flags = fcntl( socket, F_GETFL, 0 );
	if( ( flags & O_NONBLOCK ) == !should_block ) { return ret; }
	ret = 0 == fcntl( socket, F_SETFL, should_block ? flags & ~O_NONBLOCK : flags | O_NONBLOCK );
#endif
	return ret;
}

handle_t socket( int af, int type, int protocol )
{
	return ::socket( af, type, protocol );
}
handle_t socket( Domain domain, TransportType transport_type ) {
	return ::socket( to_native(domain), to_native(transport_type), 0 );
}

int close_socket( handle_t handle ) noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	return ::closesocket( handle );
#else
	return ::close( handle ); // in linux, a socket is just another file descriptor
#endif
}

int getLastSocketError() noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool waInit() noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	// https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsastartup

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	WORD    wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData {};

	int err = WSAStartup( wVersionRequested, &wsaData );
	if( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		std::printf( "WSAStartup failed with error: %d\n", err );
		WSACleanup();
		return false;
	}
#endif
	return true; // on linux we don't have to initialize anything
}

int to_native( Domain domain )
{
	switch( domain ) {
		case mart::nw::socks::port_layer::Domain::local: return AF_UNIX; break;
		case mart::nw::socks::port_layer::Domain::inet: return AF_INET; break;
		case mart::nw::socks::port_layer::Domain::inet6: return AF_INET6; break;
	}
	assert( false );
	return -1;
}

int to_native( TransportType transport_type )
{
	switch( transport_type ) {
		case mart::nw::socks::port_layer::TransportType::stream: return SOCK_STREAM; break;
		case mart::nw::socks::port_layer::TransportType::datagram: return SOCK_DGRAM; break;
		case mart::nw::socks::port_layer::TransportType::seqpacket: return SOCK_SEQPACKET; break;
	}
	assert( false );
	return -1;
}

namespace {

int to_native( SocketOptionLevel level )
{
	switch( level ) {
		case mart::nw::socks::port_layer::SocketOptionLevel::sol_socket: return SOL_SOCKET; break;
	}
	assert( false );
	return static_cast<int>( level );
}

int to_native( SocketOption option )
{
	switch( option ) {
		case mart::nw::socks::port_layer::SocketOption::so_rcvtimeo: return SO_RCVTIMEO; break;
		case mart::nw::socks::port_layer::SocketOption::so_sndtimeo: return SO_SNDTIMEO; break;
	}
	assert( false );
	return static_cast<int>( option );
}

} // namespace

int setsockopt( handle_t handle, int level, int optname, const char* optval, address_len_t optlen ) noexcept
{
	return ::setsockopt( handle, level, optname, optval, optlen );
}

int getsockopt( handle_t handle, int level, int optname, char* optval, address_len_t* optlen ) noexcept
{
	return ::getsockopt( handle, level, optname, optval, optlen );
}

int setsockopt(
	handle_t handle, SocketOptionLevel level, SocketOption optname, const char* optval, address_len_t optlen ) noexcept
{
	return ::setsockopt( handle, to_native( level ), to_native( optname ), optval, optlen );
}
int getsockopt( handle_t handle, SocketOptionLevel level, SocketOption optname, char* optval, address_len_t* optlen ) noexcept
{
	return ::getsockopt( handle, to_native( level ), to_native( optname ), optval, optlen );
}

const ::sockaddr* to_native( const sockaddr* addr )
{
	return static_cast<const ::sockaddr*>( static_cast<const void*>( addr ) );
}

::sockaddr* to_native( sockaddr* addr )
{
	return static_cast<::sockaddr*>( static_cast<void*>( addr ) );
}

int bind( handle_t s, const sockaddr* addr, address_len_t namelen ) noexcept
{
	return ::bind( s, to_native( addr ), namelen );
}

int connect( handle_t handle, const sockaddr* addr, address_len_t namelen ) noexcept
{
	return ::connect( handle, to_native( addr ), namelen );
}

int listen( handle_t sockfd, int backlog ) noexcept
{
	return ::listen( sockfd, backlog );
}

handle_t accept( handle_t sockfd, sockaddr* addr, address_len_t* addrlen ) noexcept
{
	return ::accept( sockfd, to_native( addr ), addrlen );
}

txrx_size_t send( handle_t handle, const char* buf, buf_size_t len, int flags ) noexcept
{
	return ::send( handle, buf, len, flags );
}

txrx_size_t
sendto( handle_t handle, const char* buf, buf_size_t len, int flags, const sockaddr* to, address_len_t tolen ) noexcept
{
	return ::sendto( handle, buf, len, flags, to_native( to ), tolen );
}

txrx_size_t recv( handle_t handle, char* buf, buf_size_t len, int flags ) noexcept
{
	return ::recv( handle, buf, len, flags );
}

txrx_size_t
recvfrom( handle_t handle, char* buf, buf_size_t len, int flags, sockaddr* from, address_len_t* fromlen ) noexcept
{
	return ::recvfrom( handle, buf, len, flags, to_native( from ), fromlen );
}

} // namespace port_layer
} // namespace socks

namespace ip {
namespace port_layer {

const char* inet_net_to_pres( int af, const void* src, char* dst, size_t size )
{
#ifdef MBA_UTILS_USE_WINSOCKS // detect windows os - use other guards if necessary
	return InetNtop( af, src, dst, size );
#else
	return inet_ntop( af, src, dst, size );
#endif
}

int inet_pres_to_net( int af, const char* src, void* dst )
{
#ifdef MBA_UTILS_USE_WINSOCKS // detect windows os - use other guards if necessary
	return InetPton( af, src, dst );
#else
	return inet_pton( af, src, dst );
#endif
}
} // namespace port_layer
} // namespace ip
} // namespace nw
} // namespace mart
