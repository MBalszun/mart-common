#include "./Socket.hpp"

/**
 * Socket.h (mart-common/nw)
 *
 * Copyright (C) 2015-2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides mart::experimental::Socket - an RAII-wrapper around the OS-socket handle
 *
 */

/* ######## INCLUDES ######### */
#include "Socket.hpp"

/* Project Includes */
#include "basic_types.hpp"
#include "port_layer.hpp"
#include <mart-common/utils.h>

/* Proprietary Library Includes */
/* Standard Library Includes */

#include <cerrno>

#ifdef MBA_UTILS_USE_WINSOCKS
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {
namespace socks {

namespace {

template<class Dur>
timeval to_timeval( Dur duration )
{
	using namespace std::chrono;
	timeval ret {};
	if( duration.count() > 0 ) {
		auto s      = duration_cast<seconds>( duration );
		ret.tv_sec  = mart::narrow<decltype( ret.tv_sec )>( s.count() );
		ret.tv_usec = mart::narrow<decltype( ret.tv_usec )>( ( duration_cast<microseconds>( duration ) - s ).count() );
	}
	return ret;
}

template<class Dur>
Dur from_timeval( timeval duration )
{
	using namespace std::chrono;
	return duration_cast<Dur>( seconds( duration.tv_sec ) + microseconds( duration.tv_usec ) );
}

// on windows, the wa system has to be initialized before sockets can be used
bool startUp()
{
	const static bool isInit = port_layer::waInit();
	return isInit;
}

} // namespace





int Socket::close() noexcept
{
	int ret = -1;
	if( isValid() ) {
		ret     = port_layer::close_socket( _handle );
		_handle = port_layer::invalid_handle;
	}
	return ret;
}

/* ###### send / rec ############### */

auto Socket::send( mart::ConstMemoryView data, int flags ) -> port_layer::txrx_size_t
{
	return port_layer::send(
		_handle, data.asConstCharPtr(), static_cast<port_layer::txrx_size_t>( data.size() ), flags );
}

auto Socket::recv( mart::MemoryView buffer, int flags ) -> std::pair<mart::MemoryView, int>
{
	auto ret
		= port_layer::recv( _handle, buffer.asCharPtr(), static_cast<port_layer::txrx_size_t>( buffer.size() ), flags );
	if( ret >= 0 ) {
		return {buffer.subview( 0, ret ), 0};
	} else {
		return {mart::MemoryView {}, errno};
	}
}



/* ###### connection related ############### */

int Socket::listen( int backlog )
{
	return port_layer::listen( _handle, backlog );
}

Socket Socket::accept()
{
	port_layer::address_len_t len = 0;
	port_layer::handle_t      h   = port_layer::accept( _handle, nullptr, &len );
	return Socket( h );
}

/* ###### Configuration ############### */
// TODO: could actually check mapping between option data type and optname

bool Socket::setBlocking( bool should_block ) noexcept
{
	if( _is_blocking == should_block ) { return true; }
	return _setBlocking_uncached( should_block );
}

using dword = std::uint32_t;

bool Socket::setTxTimeout( std::chrono::microseconds timeout ) noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS

	dword to_ms = static_cast<dword>( std::chrono::duration_cast<std::chrono::milliseconds>( timeout ).count() );
	auto  timeout_native = to_ms;
#else
	auto to  = to_timeval( timeout );
	auto timeout_native= to;
#endif
	auto res = this->setsockopt( SocketOptionLevel::sol_socket, SocketOption::so_sndtimeo, timeout_native );
	return res != port_layer::socket_error_value;
}

bool Socket::setRxTimeout( std::chrono::microseconds timeout ) noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	dword to_ms = static_cast<dword>( std::chrono::duration_cast<std::chrono::milliseconds>( timeout ).count() );
	auto  timeout_native = to_ms;
#else
	auto to  = to_timeval( timeout );
	auto timeout_native   = to;
#endif
	auto res = this->setsockopt( SocketOptionLevel::sol_socket, SocketOption::so_rcvtimeo, timeout_native );
	return res != port_layer::socket_error_value;
}
std::chrono::microseconds Socket::getTxTimeout() noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	dword timeout {};
#else
	timeval timeout {};
#endif

	this->getsockopt( SocketOptionLevel::sol_socket, SocketOption::so_sndtimeo, timeout );

	using namespace std::chrono;
#ifdef MBA_UTILS_USE_WINSOCKS
	return microseconds( milliseconds( timeout ) );
#else
	return from_timeval<microseconds>( timeout );
#endif
}

std::chrono::microseconds Socket::getRxTimeout() noexcept
{
#ifdef MBA_UTILS_USE_WINSOCKS
	dword timeout {};
#else
	timeval timeout {};
#endif

	this->getsockopt( SocketOptionLevel::sol_socket, SocketOption::so_rcvtimeo, timeout );

	using namespace std::chrono;
#ifdef MBA_UTILS_USE_WINSOCKS
	return microseconds( milliseconds( timeout ) );
#else
	return from_timeval<microseconds>( timeout );
#endif
}

bool Socket::_open( port_layer::Domain domain, port_layer::TransportType type )
{
	if( startUp() == false ) { return false; }
	_handle = port_layer::socket( domain, type );
	_setBlocking_uncached( true );
	return _handle != port_layer::invalid_handle;
}

bool Socket::_setBlocking_uncached( bool should_block )
{
	if( port_layer::set_blocking( _handle, should_block ) ) {
		_is_blocking = should_block;
		return true;
	}
	return false;
}

} // namespace socks
} // namespace nw
} // namespace mart
