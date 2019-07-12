#include <mart-netlib/port_layer.hpp>

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
#include <cstring> // memcpy
#include <new>     // launder
#include <type_traits>

#ifdef __cpp_lib_launder
#define MBA_LAUNDER( x ) std::launder( x )
#else
#define MBA_LAUNDER( x ) (x)
#endif


// Include OS-specific headers
#ifdef MBA_UTILS_USE_WINSOCKS

/* ######## WINDOWS ######### */

// Including Windows.h (indirectly) tends to import some nasty macros. in particular min and max
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <afunix.h>
#pragma comment( lib, "Ws2_32.lib" )

#else
#include <arpa/inet.h>
#include <cerrno>
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

using address_len_t = int;
using buffer_len_t  = int;

#else

using address_len_t = socklen_t;
using buffer_len_t  = int;

#endif // MBA_UTILS_USE_WINSOCKS

namespace {
template<class E>
using utype_t = typename std::underlying_type<E>::type; // fill in for c++14 std::unterlying_type_t
template<class T>
using rm_const_t = typename std::remove_const<T>::type; // fill in for c++14 std::unterlying_type_t

template<class E>
constexpr utype_t<E> to_utype( E e )
{
	return static_cast<utype_t<E>>( e );
}

template<class Dest, class Src>
constexpr Dest narrow_cast( Src src )
{
	return static_cast<Dest>( src );
}

constexpr address_len_t to_native_addr_len( std::size_t size )
{
	return narrow_cast<address_len_t>( size );
}

constexpr address_len_t to_native_buf_len( std::size_t size )
{
	return narrow_cast<buffer_len_t>( size );
}

} // namespace

// Define aliases for platform specific types and values
#ifdef MBA_UTILS_USE_WINSOCKS

static_assert( std::is_same<utype_t<handle_t>, SOCKET>::value, "" );
static_assert( std::is_same<utype_t<std::remove_const_t<decltype( handle_t::Invalid )>>, decltype( INVALID_SOCKET )>::value,
			   "" );
static_assert( std::is_same<rm_const_t<decltype( socket_error_value )>, decltype( SOCKET_ERROR )>::value,
			   "" );

static_assert( to_utype( handle_t::Invalid ) == INVALID_SOCKET, "" );
static_assert( socket_error_value == SOCKET_ERROR, "" );
#else

static_assert( std::is_same<utype_t<handle_t>, int>::value, "" );

static_assert( std::is_same<utype_t<rm_const_t<decltype( handle_t::Invalid )>>, decltype( -1 )>::value,
	"" );
static_assert( to_utype( handle_t::Invalid ) == -1, "" );

static_assert( std::is_same<rm_const_t<decltype( socket_error_value )>, decltype( -1 )>::value, "" );
static_assert( socket_error_value == -1, "" );

#endif // MBA_UTILS_USE_WINSOCKS

// Wrapper functions for socket related functions, that are specific to a certain platform
bool set_blocking( handle_t socket, bool should_block ) noexcept
{
	bool ret           = true;
	auto native_handle = to_utype( socket );
#ifdef MBA_UTILS_USE_WINSOCKS
	// from
	// http://stackoverflow.com/questions/5489562/in-win32-is-there-a-way-to-test-if-a-socket-is-non-blocking/33087879
	/// @note windows sockets are created in blocking mode by default
	// currently on windows, there is no easy way to obtain the socket's current blocking mode since WSAIsBlocking was
	// deprecated
	u_long non_blocking = should_block ? 0 : 1;
	ret                 = NO_ERROR == ioctlsocket( native_handle, FIONBIO, &non_blocking );
#else
	const int flags = fcntl( native_handle, F_GETFL, 0 );
	if( ( flags & O_NONBLOCK ) == !should_block ) { return ret; }
	ret = 0 == fcntl( native_handle, F_SETFL, should_block ? flags & ~O_NONBLOCK : flags | O_NONBLOCK );
#endif
	return ret;
}

namespace {
// TODO: throw exception on failure?
void startup()
{
#pragma warning( disable : 5051  ) // unknown attribute
	[[maybe_unused]] const static bool isInit = port_layer::waInit();
#pragma warning( default : 5051 )
}
} // namespace

handle_t socket( Domain domain, TransportType transport_type, Protocol protocol )
{
	startup();
	return static_cast<handle_t>(::socket( to_native( domain ), to_native( transport_type ), to_native( protocol ) ));
}

int close_socket( handle_t handle ) noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	return ::closesocket( to_utype( handle ) );
#else
	return ::close( to_utype( handle ) ); // in linux, a socket is just another file descriptor
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
		case mart::nw::socks::Domain::Invalid: return -1; break;
		case mart::nw::socks::Domain::Local: return AF_UNIX; break;
		case mart::nw::socks::Domain::Inet: return AF_INET; break;
		case mart::nw::socks::Domain::Inet6: return AF_INET6; break;
	}
	assert( false );
	return -1;
}

int to_native( TransportType transport_type )
{
	switch( transport_type ) {
		case mart::nw::socks::TransportType::stream: return SOCK_STREAM; break;
		case mart::nw::socks::TransportType::datagram: return SOCK_DGRAM; break;
		case mart::nw::socks::TransportType::seqpacket: return SOCK_SEQPACKET; break;
	}
	assert( false );
	return -1;
}

int to_native( Protocol protocol )
{
	switch( protocol ) {
		case mart::nw::socks::Protocol::Default: return 0;
		case mart::nw::socks::Protocol::Udp: return IPPROTO_UDP;
		case mart::nw::socks::Protocol::Tcp: return IPPROTO_TCP;
	}
	assert( false );
	return static_cast<int>( protocol );
}

namespace {

int to_native( SocketOptionLevel level )
{
	switch( level ) {
		case mart::nw::socks::SocketOptionLevel::sol_socket: return SOL_SOCKET; break;
	}
	assert( false );
	return static_cast<int>( level );
}

int to_native( SocketOption option )
{
	switch( option ) {
		case mart::nw::socks::SocketOption::so_rcvtimeo: return SO_RCVTIMEO; break;
		case mart::nw::socks::SocketOption::so_sndtimeo: return SO_SNDTIMEO; break;
	}
	assert( false );
	return static_cast<int>( option );
}

} // namespace

int setsockopt( handle_t handle, SocketOptionLevel level, SocketOption optname, const byte_range data ) noexcept
{
	return ::setsockopt( to_native( handle ),
						 to_native( level ),
						 to_native( optname ),
						 data.char_ptr(),
						 narrow_cast<int>( data.size() ) );
}

int getsockopt( handle_t handle, SocketOptionLevel level, SocketOption optname, byte_range_mut& data ) noexcept
{
	auto len = to_native_addr_len( data.size() );

	const auto ret
		= ::getsockopt( to_native( handle ), to_native( level ), to_native( optname ), data.char_ptr(), &len );
	data._size = len;
	return ret;
}

const ::sockaddr* to_native( const Sockaddr* addr )
{
	return static_cast<const ::sockaddr*>( static_cast<const void*>( addr ) );
}

::sockaddr* to_native( Sockaddr* addr )
{
	return static_cast<::sockaddr*>( static_cast<void*>( addr ) );
}

int bind( handle_t handle, const Sockaddr& addr ) noexcept
{
	return ::bind( to_native( handle ), addr.to_native_ptr(), to_native_addr_len( addr.size() ) );
}

int connect( handle_t handle, const Sockaddr& addr ) noexcept
{
	return ::connect( to_native( handle ), addr.to_native_ptr(), to_native_addr_len( addr.size() ) );
}

int listen( handle_t handle, int backlog ) noexcept
{
	return ::listen( to_native( handle ), backlog );
}

handle_t accept( handle_t handle, Sockaddr& addr ) noexcept
{
	auto addr_len = to_native_addr_len( addr.size() );
	auto ret      = static_cast<handle_t>(::accept( to_native( handle ), addr.to_native_ptr(), &addr_len ));
	addr.set_valid_data_range( addr_len );
	return ret;
}

handle_t accept( handle_t handle ) noexcept
{
	return static_cast<handle_t>(::accept( to_native( handle ), nullptr, 0 ));
}

txrx_size_t send( handle_t handle, byte_range buf, int flags ) noexcept
{
	return ::send( to_native( handle ), buf.char_ptr(), to_native_buf_len( buf.size() ), flags );
}

txrx_size_t sendto( handle_t handle, byte_range buf, int flags, const Sockaddr& to ) noexcept
{
	return ::sendto( to_native( handle ),
					 buf.char_ptr(),
					 to_native_buf_len( buf.size() ),
					 flags,
					 to.to_native_ptr(),
					 to_native_addr_len( to.size() ) );
}

txrx_size_t recv( handle_t handle, byte_range_mut buf, int flags ) noexcept
{
	return ::recv( to_native( handle ), buf.char_ptr(), to_native_buf_len( buf.size() ), flags );
}

txrx_size_t recvfrom( handle_t handle, byte_range_mut buf, int flags, Sockaddr& from ) noexcept
{
	auto from_len = to_native_addr_len( from.size() );
	auto ret      = ::recvfrom(
        to_native( handle ), buf.char_ptr(), to_native_buf_len( buf.size() ), flags, from.to_native_ptr(), &from_len );
	from.set_valid_data_range( from_len );
	return ret;
}

// implementation details for timeout related functions
// Todo: move into general utilities
namespace {

template<class Dur>
timeval to_timeval( Dur duration )
{
	using namespace std::chrono;
	timeval ret {};
	if( duration.count() > 0 ) {
		auto s      = duration_cast<seconds>( duration );
		ret.tv_sec  = narrow_cast<decltype( ret.tv_sec )>( s.count() );
		ret.tv_usec = narrow_cast<decltype( ret.tv_usec )>( ( duration_cast<microseconds>( duration - s ) ).count() );
	}
	return ret;
}

template<class Dur>
Dur from_timeval( timeval duration )
{
	using namespace std::chrono;
	return duration_cast<Dur>( seconds( duration.tv_sec ) + microseconds( duration.tv_usec ) );
}

using dword = std::uint32_t;
} // namespace

bool set_timeout( handle_t handle, Direction direction, std::chrono::microseconds timeout ) noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	dword to_ms = static_cast<dword>( std::chrono::duration_cast<std::chrono::milliseconds>( timeout ).count() );
	auto  timeout_native = to_ms;
#else
	auto    to             = to_timeval( timeout );
	auto    timeout_native = to;
#endif
	auto option_name = direction == Direction::Tx ? SocketOption::so_sndtimeo : SocketOption::so_rcvtimeo;
	auto res = setsockopt( handle, SocketOptionLevel::sol_socket, option_name, byte_range_from_pod( timeout_native ) );
	return res != port_layer::socket_error_value;
}

std::chrono::microseconds get_timeout( handle_t handle, Direction direction ) noexcept
{
	using namespace std::chrono;

	microseconds ret;
	const auto   option_name = direction == Direction::Tx ? SocketOption::so_sndtimeo : SocketOption::so_rcvtimeo;

#ifdef MBA_UTILS_USE_WINSOCKS
	dword native_timeout {};
#else
	timeval native_timeout {};
#endif

	auto bytes = byte_range_from_pod( native_timeout );
	auto res   = getsockopt( handle, SocketOptionLevel::sol_socket, option_name, bytes );

	if( res == port_layer::socket_error_value && bytes.size() == sizeof( native_timeout ) ) {
		ret = duration_cast<microseconds>( std::chrono::seconds( -1 ) );
	} else {
#ifdef MBA_UTILS_USE_WINSOCKS
		ret = microseconds( milliseconds( native_timeout ) );
#else
		ret = from_timeval<microseconds>( native_timeout );
#endif
	}
	return ret;
}

sockaddr_in::sockaddr_in( const ::sockaddr_in& native ) noexcept
	: sockaddr_in::sockaddr_in()
{
	static_assert( sizeof( _storage ) >= sizeof( native ),"" );
	static_assert( alignof( sockaddr_in::Storage ) >= alignof( ::sockaddr_in ),"" );

	new( _storage.raw_bytes )::sockaddr_in( native );
}

namespace {

::sockaddr_in make_sockaddr_in( mart::nw::uint32_net_t address, mart::nw::uint16_net_t port )
{
	::sockaddr_in native {};
	native.sin_family      = AF_INET;
	native.sin_addr.s_addr = to_utype( address );
	native.sin_port        = to_utype( port );
	return native;
}

} // namespace

sockaddr_in::sockaddr_in( mart::nw::uint32_net_t address, mart::nw::uint16_net_t port ) noexcept
	: sockaddr_in::sockaddr_in( make_sockaddr_in( address, port ) )
{
}

const ::sockaddr_in& sockaddr_in::native() const noexcept
{
	return *MBA_LAUNDER( reinterpret_cast<const ::sockaddr_in*>( _storage.raw_bytes ) );
}

::sockaddr_in& sockaddr_in::native() noexcept
{
	return *MBA_LAUNDER( reinterpret_cast<::sockaddr_in*>( _storage.raw_bytes ) );
}

mart::nw::uint32_net_t sockaddr_in::address() const noexcept
{
	return static_cast<mart::nw::uint32_net_t>(native().sin_addr.s_addr);
}
mart::nw::uint16_net_t sockaddr_in::port() const noexcept
{
	return static_cast<mart::nw::uint16_net_t>( native().sin_port );
}

sockaddr_in6::sockaddr_in6( const ::sockaddr_in6& native )
	: sockaddr_in6::sockaddr_in6()
{
	static_assert( sizeof( _storage ) >= sizeof( native ),"" );
	static_assert( alignof( sockaddr_in6::Storage ) >= alignof( ::sockaddr_in6 ),"" );

	std::memcpy( _storage.raw_bytes, &native, sizeof( native ) );
}

const char* inet_net_to_pres( Domain af, const void* src, char* dst, size_t size )
{
#ifdef MBA_UTILS_USE_WINSOCKS // detect windows os - use other guards if necessary
	return InetNtop( to_native( af ), src, dst, size );
#else
	return inet_ntop( to_native( af ), src, dst, size );
#endif
}

int inet_pres_to_net( Domain af, const char* src, void* dst )
{
#ifdef MBA_UTILS_USE_WINSOCKS // detect windows os - use other guards if necessary
	return InetPton( to_native( af ), src, dst );
#else
	return inet_pton( to_native( af ), src, dst );
#endif
}
} // namespace port_layer
} // namespace socks
} // namespace nw
} // namespace mart
