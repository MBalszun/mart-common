#ifndef LIB_MART_COMMON_GUARD_NW_SOCKET_H
#define LIB_MART_COMMON_GUARD_NW_SOCKET_H
/**
 * Socket.hpp (mart-common/nw)
 *
 * Copyright (C) 2015-2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides mart::Socket - an RAII-wrapper around the OS-socket handle
 *
 */

/* ######## INCLUDES ######### */

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>
#include <mart-common/utils.h>

/* Project Includes */
#include "basic_types.hpp"
#include "port_layer.hpp"
/* Standard Library Includes */
#include <cerrno>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {

namespace socks {

using port_layer::Domain;
using port_layer::SocketOption;
using port_layer::SocketOptionLevel;
using port_layer::TransportType;

/* This class is a very thin RAII wrapper around the OS's native socket handle.
 * It also translates the native socket function (like bind, connect send etc.) into member functions
 * which sometimes use a little more convenient parameter types (e.g. ArrayView instead of pointer+length)
 * It doesn't retain any state except the handle and may cache a few config flags.
 */
class Socket {
	template<class T>
	static port_layer::sockaddr* asSockAddrPtr( T& addr )
	{
		return static_cast<port_layer::sockaddr*>( static_cast<void*>( &addr ) );
	}

	template<class T>
	static const port_layer::sockaddr* asSockAddrPtr( const T& addr )
	{
		return static_cast<const port_layer::sockaddr*>( static_cast<const void*>( &addr ) );
	}

public:
	/*##### CTORS / DTORS #####*/
	constexpr Socket() = default;
	explicit Socket( port_layer::handle_t handle )
		: _handle( handle )
	{
		_setBlocking_uncached( true );
	}
	Socket( Domain domain, TransportType type ) { _open( domain, type ); }
	~Socket() { close(); }

	/*##### Special member functions #####*/
	Socket( const Socket& other ) = delete;
	Socket& operator=( const Socket& other ) = delete;

	Socket( Socket&& other ) noexcept
		: _handle {std::exchange( other._handle, port_layer::invalid_handle )}
		, _is_blocking {std::exchange( other._is_blocking, true )}
	{
	}

	Socket& operator=( Socket&& other ) noexcept
	{
		close();
		_handle      = std::exchange( other._handle, port_layer::invalid_handle );
		_is_blocking = std::exchange( other._is_blocking, true );
		return *this;
	}

	/*##### Socket operations #####*/
	bool isValid() const noexcept { return _handle != port_layer::invalid_handle; }

	int close() noexcept;

	port_layer::handle_t getNative() const { return _handle; }

	port_layer::handle_t release() { return std::exchange( _handle, port_layer::invalid_handle ); }

	/* ###### send / rec ############### */

	auto send( mart::ConstMemoryView data, int flags = 0 ) -> port_layer::txrx_size_t;
	auto recv( mart::MemoryView buffer, int flags = 0 ) -> std::pair<mart::MemoryView, int>;

	template<class AddrT>
	auto sendto( mart::ConstMemoryView data, int flags, const AddrT& addr ) -> port_layer::txrx_size_t
	{
		return port_layer::sendto( _handle,
								   data.asConstCharPtr(),
								   static_cast<port_layer::txrx_size_t>( data.size() ),
								   flags,
								   asSockAddrPtr( addr ),
								   sizeof( addr ) );
	}

	template<class AddrT>
	auto recvfrom( mart::MemoryView buffer, int flags, AddrT& src_addr ) -> std::pair<mart::MemoryView, int>
	{
		port_layer::address_len_t len = sizeof( src_addr );
		port_layer::txrx_size_t   ret = port_layer::recvfrom( _handle,
                                                            buffer.asCharPtr(),
                                                            static_cast<port_layer::txrx_size_t>( buffer.size() ),
                                                            flags,
                                                            asSockAddrPtr( src_addr ),
                                                            &len );
		if( ret >= 0 && len == sizeof( src_addr ) ) {
			return {buffer.subview( 0, ret ), 0};
		} else {
			return {mart::MemoryView {}, errno};
		}
	}

	/* ###### connection related ############### */

	template<class AddrT>
	int bind( const AddrT& addr ) noexcept
	{
		return port_layer::bind( _handle, asSockAddrPtr( addr ), sizeof( addr ) );
	}

	template<class AddrT>
	int connect( const AddrT& addr ) noexcept
	{
		return port_layer::connect( _handle, asSockAddrPtr( addr ), sizeof( addr ) );
	}

	int listen( int backlog = 10 );

	template<class AddrT>
	Socket accept( AddrT& remote_addr )
	{
		port_layer::address_len_t len = sizeof( remote_addr );

		Socket h {port_layer::accept( _handle, asSockAddrPtr( remote_addr ), &len )};
		if( h.isValid() && ( len == sizeof( remote_addr ) ) ) {
			return h;
		} else {
			return Socket {};
		}
	}

	Socket accept();

	/* ###### Configuration ############### */
	// TODO: could actually check mapping between option data type and optname
	template<class T>
	int setsockopt( SocketOptionLevel level, SocketOption optname, const T& option_data ) noexcept
	{
		auto opmem = mart::view_bytes( option_data );
		return port_layer::setsockopt(
			_handle, level, optname, opmem.asConstCharPtr(), static_cast<port_layer::txrx_size_t>( opmem.size() ) );
	}

	template<class T>
	int getsockopt( SocketOptionLevel level, SocketOption optname, T& option_data ) noexcept
	{
		port_layer::address_len_t optlen = sizeof( option_data );
		return port_layer::getsockopt( _handle, level, optname, reinterpret_cast<char*>( &option_data ), &optlen );
	}

	bool setBlocking( bool should_block ) noexcept;

	bool setTxTimeout( std::chrono::microseconds timeout ) noexcept;
	bool setRxTimeout( std::chrono::microseconds timeout ) noexcept;

	std::chrono::microseconds getTxTimeout() noexcept;
	std::chrono::microseconds getRxTimeout() noexcept;

	bool isBlocking() const
	{
		// XXX: acutally query the native socket
		return _is_blocking && isValid();
	}

private:
	bool _open( port_layer::Domain domain, port_layer::TransportType type );
	bool _setBlocking_uncached( bool should_block );

	port_layer::handle_t _handle      = port_layer::invalid_handle;
	bool                 _is_blocking = true;
};

} // namespace socks
} // namespace nw
} // namespace mart

#endif
