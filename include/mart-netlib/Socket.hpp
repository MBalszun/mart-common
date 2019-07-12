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
 * @brief:	Provides mart::Socket - an thin RAII-wrapper around the port_layer functions
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
#include "basic_types.hpp"
#include "port_layer.hpp"

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>

/* Standard Library Includes */
#include <cerrno>
#include <chrono>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {
namespace socks {
namespace _detail_socket_ {
inline byte_range to_byte_range( const ::mart::ConstMemoryView memory )
{
	return byte_range {reinterpret_cast<unsigned char const*>( memory.data() ), memory.size()};
}

inline byte_range_mut to_mutable_byte_range( ::mart::MemoryView memory )
{
	return byte_range_mut {reinterpret_cast<unsigned char*>( memory.data() ), memory.size()};
}
} // namespace _detail_socket_

/* This class is a very thin RAII wrapper around the OS's native socket handle.
 * It also translates the native socket function (like bind, connect send etc.) into member functions
 * which sometimes use a little more convenient parameter types (e.g. ArrayView instead of pointer+length)
 * It doesn't retain any state except the handle and may cache a few config flags.
 */
class Socket {
	//template<class T>
	//static ::sockaddr* asSockAddrPtr( T& addr )
	//{
	//	return static_cast<Sockaddr*>( static_cast<void*>( &addr ) );
	//}

	//template<class T>
	//static const ::sockaddr* asSockAddrPtr( const T& addr )
	//{
	//	return static_cast<const Sockaddr*>( static_cast<const void*>( &addr ) );
	//}

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
		: _handle {std::exchange( other._handle, port_layer::handle_t::Invalid )}
		, _is_blocking {std::exchange( other._is_blocking, false )}
	{
	}

	Socket& operator=( Socket&& other ) noexcept
	{
		close();
		_handle      = std::exchange( other._handle, port_layer::handle_t::Invalid );
		_is_blocking = std::exchange( other._is_blocking, true );
		return *this;
	}

	/*##### Socket operations #####*/
	bool isValid() const noexcept { return _handle != port_layer::handle_t::Invalid; }

	int close() noexcept
	{
		int ret = -1;
		if( isValid() ) {
			ret     = port_layer::close_socket( _handle );
			_handle = port_layer::handle_t::Invalid;
		}
		return ret;
	}

	port_layer::handle_t getNative() const { return _handle; }

	port_layer::handle_t release() { return std::exchange( _handle, port_layer::handle_t::Invalid ); }

	/* ###### send / rec ############### */

	auto send( mart::ConstMemoryView data, int flags ) -> txrx_size_t
	{
		return port_layer::send( _handle, _detail_socket_::to_byte_range( data ), flags );
	}

	auto recv( mart::MemoryView buffer, int flags ) -> std::pair<mart::MemoryView, int>
	{
		auto ret = port_layer::recv( _handle, _detail_socket_::to_mutable_byte_range( buffer ), flags );
		if( ret >= 0 ) {
			return {buffer.subview( 0, ret ), 0};
		} else {
			return {mart::MemoryView {}, errno};
		}
	}

	auto sendto( mart::ConstMemoryView data, int flags, const Sockaddr& addr ) -> txrx_size_t
	{
		return port_layer::sendto( _handle, _detail_socket_::to_byte_range( data ), flags, addr );
	}

	auto recvfrom( mart::MemoryView buffer, int flags, Sockaddr& src_addr ) -> std::pair<mart::MemoryView, int>
	{
		auto ret = port_layer::recvfrom( _handle, _detail_socket_::to_mutable_byte_range( buffer ), flags, src_addr );
		if( ret >= 0 ) {
			return {buffer.subview( 0, ret ), 0};
		} else {
			return {mart::MemoryView {}, errno};
		}
	}

	/* ###### connection related ############### */

	int bind( const Sockaddr& addr ) noexcept { return port_layer::bind( _handle, addr ); }

	int connect( const Sockaddr& addr ) noexcept { return port_layer::connect( _handle, addr ); }

	int listen( int backlog ) { return port_layer::listen( _handle, backlog ); }

	Socket accept() { return Socket( port_layer::accept( _handle ) ); }

	Socket accept( Sockaddr& remote_addr ) { return Socket {port_layer::accept( _handle, remote_addr )}; }

	/* ###### Configuration ############### */
	template<class T>
	int setsockopt( SocketOptionLevel level, SocketOption optname, const T& option_data ) noexcept
	{
		auto opmem = byte_range_from_pod( option_data );
		return port_layer::setsockopt( _handle, level, optname, opmem );
	}

	template<class T>
	int getsockopt( SocketOptionLevel level, SocketOption optname, T& option_data ) noexcept
	{
		return port_layer::getsockopt( _handle, level, optname, byte_range_from_pod( option_data ) );
	}

	bool setBlocking( bool should_block ) noexcept
	{
		if( _is_blocking == should_block ) { return true; }
		return _setBlocking_uncached( should_block );
	}
	bool isBlocking() const
	{
		// XXX: acutally query the native socket
		return _is_blocking && isValid();
	}

	bool setTxTimeout( std::chrono::microseconds timeout ) noexcept
	{
		return port_layer::set_timeout( _handle, Direction::Tx, timeout );
	}

	bool setRxTimeout( std::chrono::microseconds timeout ) noexcept
	{
		return port_layer::set_timeout( _handle, Direction::Rx, timeout );
	}
	std::chrono::microseconds getTxTimeout() noexcept { return port_layer::get_timeout( _handle, Direction::Tx ); }

	std::chrono::microseconds getRxTimeout() noexcept { return port_layer::get_timeout( _handle, Direction::Rx ); }

private:
	bool _open( Domain domain, TransportType type )
	{
		_handle = port_layer::socket( domain, type );
		_setBlocking_uncached( true );
		return _handle != port_layer::handle_t::Invalid;
	}

	bool _setBlocking_uncached( bool should_block )
	{
		if( port_layer::set_blocking( _handle, should_block ) ) {
			_is_blocking = should_block;
			return true;
		}
		return false;
	}

	port_layer::handle_t _handle      = port_layer::handle_t::Invalid;
	bool                 _is_blocking = true;
};

} // namespace socks
} // namespace nw
} // namespace mart

#endif
