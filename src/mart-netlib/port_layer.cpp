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
#define MBA_LAUNDER( x ) ( x )
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
#include <netdb.h> //addrinfo
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h> //close
#endif
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

#ifdef __has_cpp_attribute
#if __has_cpp_attribute( maybe_unused )
#define MART_NETLIB_PORT_LAYER_MAYBE_UNUSED [[maybe_unused]]
#else
#define MART_NETLIB_PORT_LAYER_MAYBE_UNUSED
#endif
#else
#define MART_NETLIB_PORT_LAYER_MAYBE_UNUSED
#endif

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

ErrorCode get_last_socket_error() noexcept
{
	return ErrorCode{ static_cast<ErrorCode::Value_t>(
#ifdef MBA_UTILS_USE_WINSOCKS
		WSAGetLastError()
#else
        errno
#endif
			) };
}

namespace {
template<class E>
using utype_t = typename std::underlying_type<E>::type; // fill in for c++14 std::unterlying_type_t
template<class T>
using rm_const_t = typename std::remove_const<T>::type; // fill in for c++14 remove_const_t

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

#if _MSC_VER
static_assert( (int)ErrorCode::Value_t::NoError == NOERROR, "" );
#else
static_assert( (int)ErrorCode::Value_t::NoError == 0, "" );
#endif
static_assert( (int)ErrorCode::Value_t::TryAgain == EAGAIN, "" );
static_assert( (int)ErrorCode::Value_t::WouldBlock == EWOULDBLOCK, "" );

ErrorCode get_appropriate_error_code( int function_result )
{
	if( function_result == 0 ) {
		return { ErrorCode::Value_t::NoError };
	} else {
		return get_last_socket_error();
	}
}

MART_NETLIB_PORT_LAYER_MAYBE_UNUSED ErrorCode get_appropriate_error_code( bool success )
{
	if( success ) {
		return { ErrorCode::Value_t::NoError };
	} else {
		return get_last_socket_error();
	}
}

template<class T>
struct non_deduced {
	using type = T;
};

template<class T>
using non_deduced_t = typename non_deduced<T>::type;

template<class T>
ReturnValue<T> make_return_value( const T error_value, non_deduced_t<T> function_result )
{
	if( function_result != error_value ) {
		return ReturnValue<T>{ function_result };
	} else {
		return ReturnValue<T>( get_last_socket_error() );
	}
}

} // namespace

// Define aliases for platform specific types and values
#ifdef MBA_UTILS_USE_WINSOCKS

static_assert( std::is_same<utype_t<handle_t>, SOCKET>::value, "" );
static_assert(
	std::is_same<utype_t<std::remove_const_t<decltype( handle_t::Invalid )>>, decltype( INVALID_SOCKET )>::value, "" );

static_assert( to_utype( handle_t::Invalid ) == INVALID_SOCKET, "" );
#else

static_assert( std::is_same<utype_t<handle_t>, int>::value, "" );

static_assert( std::is_same<utype_t<rm_const_t<decltype( handle_t::Invalid )>>, decltype( -1 )>::value, "" );
static_assert( to_utype( handle_t::Invalid ) == -1, "" );

#endif // MBA_UTILS_USE_WINSOCKS

// Wrapper functions for socket related functions, that are specific to a certain platform
ErrorCode set_blocking( handle_t socket, bool should_block ) noexcept
{
	auto native_handle = to_utype( socket );
#ifdef MBA_UTILS_USE_WINSOCKS
	// from
	// http://stackoverflow.com/questions/5489562/in-win32-is-there-a-way-to-test-if-a-socket-is-non-blocking/33087879
	/// @note windows sockets are created in blocking mode by default
	// currently on windows, there is no easy way to obtain the socket's current blocking mode since WSAIsBlocking was
	// deprecated
	u_long     non_blocking = should_block ? 0 : 1;
	const auto ret          = ioctlsocket( native_handle, FIONBIO, &non_blocking );
#else
	const int  flags = fcntl( native_handle, F_GETFL, 0 );
	const auto ret   = fcntl( native_handle, F_SETFL, should_block ? flags & ~O_NONBLOCK : flags | O_NONBLOCK );
#endif
	return get_appropriate_error_code( ret );
}

// TODO: throw exception on failure?
bool startup()
{
	MART_NETLIB_PORT_LAYER_MAYBE_UNUSED const static bool isInit = port_layer::waInit();
	return isInit;
}

ReturnValue<handle_t> socket( Domain domain, TransportType transport_type, Protocol protocol ) noexcept
{
	startup();
	return make_return_value(
		handle_t::Invalid,
		static_cast<handle_t>( ::socket( to_native( domain ), to_native( transport_type ), to_native( protocol ) ) ) );
}

ErrorCode close_socket( handle_t handle ) noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	return get_appropriate_error_code( ::closesocket( to_utype( handle ) ) );
#else
	return get_appropriate_error_code(
		::close( to_utype( handle ) ) ); // in linux, a socket is just another file descriptor
#endif
}

bool waInit() noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	// https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-wsastartup

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	WORD    wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData{};

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

int to_native( Domain domain ) noexcept
{
	switch( domain ) {
		case mart::nw::socks::Domain::Invalid: return -1;
		case mart::nw::socks::Domain::Local: return AF_UNIX;
		case mart::nw::socks::Domain::Inet: return AF_INET;
		case mart::nw::socks::Domain::Inet6: return AF_INET6;
		case mart::nw::socks::Domain::Unspec: return AF_UNSPEC;
	}
	assert( false );
	return -1;
}

mart ::nw::socks::ReturnValue<mart ::nw::socks::Domain> from_native_domain( int domain ) noexcept
{
	using RType = mart ::nw::socks::ReturnValue<mart::nw::socks::Domain>;
	switch( domain ) {
		case -1: return RType{ mart ::nw::socks::Domain::Invalid };
		case AF_UNIX: return RType{ mart::nw::socks::Domain::Local };
		case AF_INET: return RType{ mart::nw::socks::Domain::Inet };
		case AF_INET6: return RType{ mart::nw::socks::Domain::Inet6 };
		case AF_UNSPEC: return RType{ mart::nw::socks::Domain::Unspec };
	}
	assert( false );
	return RType{ ErrorCodeValues::InvalidArgument };
}

int to_native( TransportType transport_type ) noexcept
{
	switch( transport_type ) {
		case mart::nw::socks::TransportType::Stream: return SOCK_STREAM;
		case mart::nw::socks::TransportType::Datagram: return SOCK_DGRAM;
		case mart::nw::socks::TransportType::Seqpacket: return SOCK_SEQPACKET;
		case mart::nw::socks::TransportType::Invalid: return 0;
	}
	assert( false );
	return -1;
}

mart::nw::socks::ReturnValue<TransportType> from_native_transport_type( int transport_type ) noexcept
{
	using RType = mart ::nw::socks::ReturnValue<mart::nw::socks::TransportType>;
	switch( transport_type ) {
		case SOCK_STREAM: return RType{ mart::nw::socks::TransportType::Stream };
		case SOCK_DGRAM: return RType{ mart::nw::socks::TransportType::Datagram };
		case SOCK_SEQPACKET: return RType{ mart::nw::socks::TransportType::Seqpacket };
	}
	assert( false );
	return RType{ ErrorCodeValues::InvalidArgument };
}

int to_native( Protocol protocol ) noexcept
{
	switch( protocol ) {
		case mart::nw::socks::Protocol::Default: return 0;
		case mart::nw::socks::Protocol::Udp: return IPPROTO_UDP;
		case mart::nw::socks::Protocol::Tcp: return IPPROTO_TCP;
	}
	assert( false );
	return static_cast<int>( protocol );
}

mart ::nw::socks::ReturnValue<mart::nw::socks::Protocol> from_native_protocol( int protocol ) noexcept
{
	using RType = mart::nw::socks::ReturnValue<mart::nw::socks::Protocol>;
	switch( protocol ) {
		case 0: return RType{ mart::nw::socks::Protocol::Default };
		case IPPROTO_UDP: return RType{ mart::nw::socks::Protocol::Udp };
		case IPPROTO_TCP: return RType{ mart::nw::socks::Protocol::Tcp };
	}
	assert( false );
	return RType{ ErrorCodeValues::InvalidArgument };
}

namespace {

int to_native( SocketOptionLevel level )
{
	switch( level ) {
		case mart::nw::socks::SocketOptionLevel::Socket: return SOL_SOCKET; break;
	}
	assert( false );
	return static_cast<int>( level );
}

int to_native( SocketOption option )
{
	switch( option ) {
		case mart::nw::socks::SocketOption::so_rcvtimeo: return SO_RCVTIMEO; break;
		case mart::nw::socks::SocketOption::so_sndtimeo: return SO_SNDTIMEO; break;
		case mart::nw::socks::SocketOption::so_reuseaddr: return SO_REUSEADDR; break;
	}
	assert( false );
	return static_cast<int>( option );
}

} // namespace

ErrorCode setsockopt( handle_t handle, SocketOptionLevel level, SocketOption optname, const byte_range data ) noexcept
{
	return get_appropriate_error_code( ::setsockopt( to_native( handle ),
													 to_native( level ),
													 to_native( optname ),
													 data.char_ptr(),
													 narrow_cast<int>( data.size() ) ) );
}

ErrorCode getsockopt( handle_t handle, SocketOptionLevel level, SocketOption optname, byte_range_mut& data ) noexcept
{
	auto len = to_native_addr_len( data.size() );

	const auto ret
		= ::getsockopt( to_native( handle ), to_native( level ), to_native( optname ), data.char_ptr(), &len );
	data._size = len;
	return get_appropriate_error_code( ret );
}

ErrorCode getsockname( handle_t handle, Sockaddr& addr ) noexcept
{
	auto       len = to_native_addr_len( addr.size() );
	const auto ret = ::getsockname( to_native( handle ), addr.to_native_ptr(), &len );
	addr.set_valid_data_range( len );
	return get_appropriate_error_code( ret );
}

const ::sockaddr* to_native( const Sockaddr* addr )
{
	return static_cast<const ::sockaddr*>( static_cast<const void*>( addr ) );
}

::sockaddr* to_native( Sockaddr* addr )
{
	return static_cast<::sockaddr*>( static_cast<void*>( addr ) );
}

ErrorCode bind( handle_t handle, const Sockaddr& addr ) noexcept
{
	return get_appropriate_error_code(
		::bind( to_native( handle ), addr.to_native_ptr(), to_native_addr_len( addr.size() ) ) );
}

ErrorCode connect( handle_t handle, const Sockaddr& addr ) noexcept
{
	return get_appropriate_error_code(
		::connect( to_native( handle ), addr.to_native_ptr(), to_native_addr_len( addr.size() ) ) );
}

ErrorCode listen( handle_t handle, int backlog ) noexcept
{
	return get_appropriate_error_code( ::listen( to_native( handle ), backlog ) );
}

ReturnValue<handle_t> accept( handle_t handle, Sockaddr& addr ) noexcept
{
	auto       addr_len = to_native_addr_len( addr.size() );
	const auto ret      = static_cast<handle_t>( ::accept( to_native( handle ), addr.to_native_ptr(), &addr_len ) );
	addr.set_valid_data_range( addr_len );
	return make_return_value( handle_t::Invalid, ret );
}

ReturnValue<handle_t> accept( handle_t handle ) noexcept
{
	return make_return_value( handle_t::Invalid, static_cast<handle_t>( ::accept( to_native( handle ), nullptr, 0 ) ) );
}

ReturnValue<txrx_size_t> send( handle_t handle, byte_range buf, int flags ) noexcept
{
#ifndef MBA_UTILS_USE_WINSOCKS
flags = flags | MSG_NOSIGNAL;
#endif // ! MBA_UTILS_USE_WINSOCKS

	return make_return_value( txrx_size_t{ -1 },
							  ::send( to_native( handle ), buf.char_ptr(), to_native_buf_len( buf.size() ), flags ) );
}

namespace {
bool is_invalid_destination_address( const Sockaddr& to )
{
	const auto* dest_ptr = to.to_native_ptr();
	// check for "0.0.0.0"
	if( dest_ptr->sa_family == AF_INET && ( (const ::sockaddr_in*)dest_ptr )->sin_addr.s_addr == 0 ) { return true; }

	// TODO: there are many more cases not covered here
	// The library doesn't really support IPv6 beyond the portlayer yet, so lets avoid the code for this alltogether
	// if( dest_ptr->sa_family == AF_INET6 ) {
	//	const auto(& addr)[16] = ( (const ::sockaddr_in6*)dest_ptr )->sin6_addr.s6_addr;
	//	for( int i = 0; i < sizeof( addr ); ++i ) {
	//		if( addr[i] != 0 ) { return false; }
	//	}
	//	return true;
	//}
	return false;
}

} // namespace

ReturnValue<txrx_size_t> sendto( handle_t handle, byte_range buf, int flags, const Sockaddr& to ) noexcept
{
	// sending messages to address 0.0.0.0 is apparently handled differently on different platforms,
	// so we have to catch it manually here
	if( is_invalid_destination_address( to ) ) {
		return ReturnValue<txrx_size_t>{ ErrorCode{ ErrorCodeValues::InvalidArgument } };
	}

	return make_return_value( txrx_size_t{ -1 },
							  ::sendto( to_native( handle ),
										buf.char_ptr(),
										to_native_buf_len( buf.size() ),
										flags,
										to.to_native_ptr(),
										to_native_addr_len( to.size() ) ) );
}

ReturnValue<txrx_size_t> recv( handle_t handle, byte_range_mut buf, int flags ) noexcept
{
	return make_return_value( txrx_size_t{ -1 },
							  ::recv( to_native( handle ), buf.char_ptr(), to_native_buf_len( buf.size() ), flags ) );
}

ReturnValue<txrx_size_t> recvfrom( handle_t handle, byte_range_mut buf, int flags, Sockaddr& from ) noexcept
{
	auto from_len = to_native_addr_len( from.size() );
	auto ret      = ::recvfrom(
        to_native( handle ), buf.char_ptr(), to_native_buf_len( buf.size() ), flags, from.to_native_ptr(), &from_len );
	from.set_valid_data_range( from_len );
	return make_return_value( txrx_size_t{ -1 }, ret );
}

// implementation details for timeout related functions
// Todo: move into general utilities
namespace {

template<class Dur>
timeval to_timeval( Dur duration )
{
	using namespace std::chrono;
	timeval ret{};
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

ErrorCode set_timeout( handle_t handle, Direction direction, std::chrono::microseconds timeout ) noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	dword to_ms = static_cast<dword>( std::chrono::duration_cast<std::chrono::milliseconds>( timeout ).count() );
	if( to_ms == 0 && timeout != std::chrono::microseconds::zero() ) { to_ms = 1; }
	auto native_timeout = to_ms;
#else
	auto    to             = to_timeval( timeout );
	auto    native_timeout = to;
#endif
	auto option_name = direction == Direction::Tx ? SocketOption::so_sndtimeo : SocketOption::so_rcvtimeo;
	return port_layer::setsockopt(
		handle, SocketOptionLevel::Socket, option_name, byte_range_from_pod( native_timeout ) );
}

ReturnValue<std::chrono::microseconds> get_timeout( handle_t handle, Direction direction ) noexcept
{
	using namespace std::chrono;

	const auto option_name = direction == Direction::Tx ? SocketOption::so_sndtimeo : SocketOption::so_rcvtimeo;

#ifdef MBA_UTILS_USE_WINSOCKS
	dword native_timeout{};
#else
	timeval native_timeout{};
#endif

	auto bytes = byte_range_from_pod( native_timeout );
	auto res   = getsockopt( handle, SocketOptionLevel::Socket, option_name, bytes );

	if( !res || bytes.size() != sizeof( native_timeout ) ) {
		return ReturnValue<std::chrono::microseconds>{ res };
	} else {
#ifdef MBA_UTILS_USE_WINSOCKS
		microseconds ret = microseconds( milliseconds( native_timeout ) );
#else
		microseconds ret = from_timeval<microseconds>( native_timeout );
#endif
		return ReturnValue<std::chrono::microseconds>( ret );
	}
}

SockaddrIn::SockaddrIn( const ::sockaddr_in& native ) noexcept
	: SockaddrIn::SockaddrIn()
{
	static_assert( sizeof( _storage ) >= sizeof( native ), "" );
	static_assert( alignof( SockaddrIn::Storage ) >= alignof( ::sockaddr_in ), "" );

	new( _storage.raw_bytes )::sockaddr_in( native );
}

namespace {

::sockaddr_in make_sockaddr_in( mart::nw::uint32_net_t address, mart::nw::uint16_net_t port )
{
	::sockaddr_in native{};
	native.sin_family      = AF_INET;
	native.sin_addr.s_addr = to_utype( address );
	native.sin_port        = to_utype( port );
	return native;
}

} // namespace

SockaddrIn::SockaddrIn( mart::nw::uint32_net_t address, mart::nw::uint16_net_t port ) noexcept
	: SockaddrIn::SockaddrIn( make_sockaddr_in( address, port ) )
{
}

const ::sockaddr_in& SockaddrIn::native() const noexcept
{
	return *MBA_LAUNDER( reinterpret_cast<const ::sockaddr_in*>( _storage.raw_bytes ) );
}

::sockaddr_in& SockaddrIn::native() noexcept
{
	return *MBA_LAUNDER( reinterpret_cast<::sockaddr_in*>( _storage.raw_bytes ) );
}

mart::nw::uint32_net_t SockaddrIn::address() const noexcept
{
	return static_cast<mart::nw::uint32_net_t>( native().sin_addr.s_addr );
}
mart::nw::uint16_net_t SockaddrIn::port() const noexcept
{
	return static_cast<mart::nw::uint16_net_t>( native().sin_port );
}

SockaddrIn6::SockaddrIn6( const ::sockaddr_in6& native )
	: SockaddrIn6::SockaddrIn6()
{
	static_assert( sizeof( _storage ) >= sizeof( native ), "" );
	static_assert( alignof( Storage ) >= alignof( ::sockaddr_in6 ), "" );

	std::memcpy( _storage.raw_bytes, &native, sizeof( native ) );
}

const ::sockaddr_in6& SockaddrIn6::native() const noexcept
{
	return *MBA_LAUNDER( reinterpret_cast<const ::sockaddr_in6*>( _storage.raw_bytes ) );
}

::sockaddr_in6& SockaddrIn6::native() noexcept
{
	return *MBA_LAUNDER( reinterpret_cast<::sockaddr_in6*>( _storage.raw_bytes ) );
}

std::array<std::uint8_t, 16> SockaddrIn6::address() const noexcept
{
	std::array<std::uint8_t, 16> ret;
	std::memcpy( ret.data(), native().sin6_addr.s6_addr, 16 );
	return ret;
}

namespace {

::sockaddr_un make_sockaddr_un( const char* u8path, std::size_t length )
{
	::sockaddr_un         native{};
	constexpr std::size_t max_unix_path_length = sizeof( native.sun_path );
	native.sun_family                          = AF_UNIX;
	std::size_t actual_length                  = length < max_unix_path_length - 1 ? length : max_unix_path_length - 1;
	if (actual_length > 0) {
		std::memcpy( native.sun_path, u8path, actual_length );
	}
	native.sun_path[actual_length] = '\0';
	return native;
}

} // namespace

SockaddrUn::SockaddrUn( const char* u8path, std::size_t length )
	: SockaddrUn( make_sockaddr_un( u8path, length ) )
{
}

SockaddrUn::SockaddrUn( const ::sockaddr_un& native )
	: SockaddrUn::SockaddrUn()
{
	static_assert( sizeof( _storage ) >= sizeof( native ), "" );
	static_assert( alignof( Storage ) >= alignof( ::sockaddr_un ), "" );

	std::memcpy( _storage.raw_bytes, &native, sizeof( native ) );
}

const char* SockaddrUn::path() const noexcept
{
	return reinterpret_cast<const ::sockaddr_un*>( &_storage )->sun_path;
}

std::size_t SockaddrUn::length() const noexcept
{
	return std::strlen( reinterpret_cast<const ::sockaddr_un*>( &_storage )->sun_path );
}

const char* inet_net_to_pres( Domain af, const void* src, char* dst, size_t size )
{
	return inet_ntop( to_native( af ), src, dst, size );
}

const char* inet_net_to_pres( const ::sockaddr_in* src, char* dst, std::size_t size )
{
	auto res = ::inet_ntop( AF_INET, &src->sin_addr, dst, size );
	if( res == nullptr ) { return nullptr; }
	auto addrlen = std::strlen( dst );

	auto portstr = std::to_string( mart::net::to_host_order( mart::net::uint16_net_t( src->sin_port ) ) );
	if( addrlen + portstr.size() + 2 + 1 > size ) {
		// can't append port if not enough space for ":", port string and null terminator
		return dst;
	}
#ifdef _MSC_VER
#pragma warning( disable : 4996 )	// Disable warning about unsafe strncat
#endif
	std::strncat( dst, ":", 2 );
	std::strncat( dst, portstr.c_str(), size - addrlen - 3 );

	return dst;
}

const char* inet_net_to_pres( const ::sockaddr_in6* src, char* dst, std::size_t size )
{
	auto res = ::inet_ntop( AF_INET6, &src->sin6_addr, dst, size );
	if( res == nullptr ) { return nullptr; }
	auto addrlen = std::strlen( dst );

	auto portstr = std::to_string( mart::net::to_host_order( mart::net::uint16_net_t( src->sin6_port ) ) );
	if( addrlen + portstr.size() + 2 + 1 > size ) {
		// can't append port if not enough space for ":", port string and null terminator
		return dst;
	}
	std::strncat( dst, ":", 2 );
	std::strncat( dst, portstr.c_str(), size - addrlen - 3 );
	return dst;
}

const char* inet_net_to_pres( const ::sockaddr* src, char* dst, std::size_t size )
{
	auto res = from_native_domain( src->sa_family );

	switch( res.value_or( mart::nw::socks::Domain::Invalid ) ) {
		case mart::nw::socks::Domain::Inet: {
			return inet_net_to_pres( reinterpret_cast<const ::sockaddr_in*>( src ), dst, size );
		}
		case mart::nw::socks::Domain::Inet6: {
			return inet_net_to_pres( reinterpret_cast<const ::sockaddr_in6*>( src ), dst, size );
		}
		case mart::nw::socks::Domain::Local:
			return std::strncpy( dst, reinterpret_cast<const ::sockaddr_un*>( src )->sun_path, size );

		case mart::nw::socks::Domain::Invalid: break;
		case mart::nw::socks::Domain::Unspec: break;
	}

	return "Invalid Domain for inet_net_to_pres";
}

std::string to_string( const ::sockaddr_in& addr )
{
	// address format: aaa.bbb.ccc.ddd:XXXXX\0
	char buffer[4 * 4 + 5 + 1];

	(void)::inet_ntop( AF_INET, &addr.sin_addr, buffer, sizeof( buffer ) );
	std::strncat( buffer, ":", 1 );
	auto portstr = std::to_string( mart::net::to_host_order( mart::net::uint16_net_t( addr.sin_port ) ) );
	std::strncat( buffer, portstr.c_str(), sizeof( buffer ) - std::strlen( buffer ) - 1 );
	return std::string( buffer );
}

std::string to_string( const ::sockaddr_in6& addr )
{
	// address format: aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh:XXXXX\0
	char buffer[8 * 5 + 5 + 1];
	(void)::inet_ntop( AF_INET6, &addr.sin6_addr, buffer, sizeof( buffer ) );
	std::strncat( buffer, ":", 1 );
	auto portstr = std::to_string( mart::net::to_host_order( mart::net::uint16_net_t( addr.sin6_port ) ) );
	std::strncat( buffer, portstr.c_str(), sizeof( buffer ) - std::strlen( buffer ) - 1 );
	return std::string( buffer );
}

std::string to_string( const ::sockaddr_un& addr )
{
	return std::string( addr.sun_path );
}

std::string to_string( const Sockaddr& addr )
{
	const auto* native = addr.to_native_ptr();

	switch( from_native_domain( native->sa_family ).value_or( mart::nw::socks::Domain::Invalid ) ) {
		case mart::nw::socks::Domain::Inet: {
			return to_string( *reinterpret_cast<const ::sockaddr_in*>( native ) );
		}
		case mart::nw::socks::Domain::Inet6: {
			return to_string( *reinterpret_cast<const ::sockaddr_in6*>( native ) );
		}
		case mart::nw::socks::Domain::Local: {
			return to_string( *reinterpret_cast<const ::sockaddr_un*>( native ) );
		}

		case mart::nw::socks::Domain::Invalid: break;
		case mart::nw::socks::Domain::Unspec: break;
	}

	return "Invalid Domain for inet_net_to_pres";
}

int inet_pres_to_net( Domain af, const char* src, void* dst )
{
	return inet_pton( to_native( af ), src, dst );
}

namespace {

::addrinfo to_native_hint( const AddrInfo& address_info ) noexcept
{
	::addrinfo ret{};
	ret.ai_family   = port_layer::to_native( address_info.family );
	ret.ai_socktype = port_layer::to_native( address_info.socktype );
	ret.ai_protocol = port_layer::to_native( address_info.protocol );

	return ret;
}

::addrinfo to_native_hint( const AddrInfoHints& address_info ) noexcept
{
	::addrinfo ret{};
	ret.ai_family   = port_layer::to_native( address_info.family );
	ret.ai_socktype = port_layer::to_native( address_info.socktype );
	ret.ai_protocol = port_layer::to_native( address_info.protocol );

	return ret;
}

AddrInfo from_native( const ::addrinfo& native )
{
	AddrInfo ret{};

	ret.flags    = native.ai_flags;
	ret.family   = port_layer::from_native_domain( native.ai_family ).value_or( Domain::Invalid );
	ret.socktype = port_layer::from_native_transport_type( native.ai_socktype )
					   .value_or( mart::net::socks::TransportType::Invalid );
	ret.protocol = port_layer::from_native_protocol( native.ai_protocol ).value_or( Protocol::Default );

	switch( from_native_domain( native.ai_addr->sa_family ).value_or( Domain::Invalid ) ) {
		case Domain::Local:
			ret.addr = std ::unique_ptr<SockaddrPolyWrapper<SockaddrUn>>( new SockaddrPolyWrapper<SockaddrUn>{
				SockaddrUn( *reinterpret_cast<const ::sockaddr_un*>( native.ai_addr ) ) } );
			break;
		case Domain::Inet6:
			ret.addr = std ::unique_ptr<SockaddrPolyWrapper<SockaddrIn6>>( new SockaddrPolyWrapper<SockaddrIn6>{
				SockaddrIn6( *reinterpret_cast<const ::sockaddr_in6*>( native.ai_addr ) ) } );
			break;
		case Domain::Inet:
			ret.addr = std ::unique_ptr<SockaddrPolyWrapper<SockaddrIn>>( new SockaddrPolyWrapper<SockaddrIn>{
				SockaddrIn( *reinterpret_cast<const ::sockaddr_in*>( native.ai_addr ) ) } );
			break;
		case Domain::Invalid: break;
		case Domain::Unspec: break;
	}

	if( native.ai_canonname ) { ret.canonname = native.ai_canonname; }

	return ret;
}

} // namespace

NonTrivialReturnValue<std::vector<AddrInfo>>
getaddrinfo( const char* node_name, const char* service_name, const AddrInfo& hints ) noexcept
{
	::addrinfo* native_addr;
	const auto  native_hint = to_native_hint( hints );
	auto        res         = ::getaddrinfo( node_name, service_name, &native_hint, &native_addr );
	if( res != 0 ) {
		return NonTrivialReturnValue<std::vector<AddrInfo>>( { static_cast<ErrorCode::Value_t>( res ) } );
	}

	std::vector<AddrInfo> ret;
	while( native_addr != nullptr ) {
		ret.push_back( from_native( *native_addr ) );
		native_addr = native_addr->ai_next;
	}

	return NonTrivialReturnValue<std::vector<AddrInfo>>( std::move( ret ) );
}

NonTrivialReturnValue<std::vector<AddrInfo>>
getaddrinfo( const char* node_name, const char* service_name, const AddrInfoHints& hints ) noexcept
{
	::addrinfo* native_addr;
	const auto  native_hint = to_native_hint( hints );
	auto        res         = ::getaddrinfo( node_name, service_name, &native_hint, &native_addr );
	if( res != 0 ) {
		return NonTrivialReturnValue<std::vector<AddrInfo>>( { static_cast<ErrorCode::Value_t>( res ) } );
	}

	std::vector<AddrInfo> ret;
	while( native_addr != nullptr ) {
		ret.push_back( from_native( *native_addr ) );
		native_addr = native_addr->ai_next;
	}

	return NonTrivialReturnValue<std::vector<AddrInfo>>( std::move( ret ) );
}

NonTrivialReturnValue<std::vector<AddrInfo>>
getaddrinfo( const char* node_name, const char* service_name, Domain addr_type ) noexcept
{
	AddrInfoHints hint{};
	hint.family   = addr_type;
	hint.flags    = AI_CANONNAME;
	hint.socktype = TransportType::Invalid; // allows for any
	return getaddrinfo( node_name, service_name, hint );
}

} // namespace port_layer
} // namespace socks
} // namespace nw
} // namespace mart
