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
#include <cstring>
#include <string_view>

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

inline byte_range to_byte_range( const std::string_view memory )
{
	return byte_range {reinterpret_cast<unsigned char const*>( memory.data() ), memory.size()};
}

} // namespace _detail_socket_

inline std::string_view to_text_rep( ErrorCode code )
{
#ifdef _MSC_VER
#pragma warning( suppress : 4996 )
	return std::string_view( std::strerror( code.raw_value() ) );
#else
	return std::string_view( std::strerror( code.raw_value() ) );
#endif
}

/* This class is a very thin RAII wrapper around the OS's native socket handle.
 * It also translates the native socket function (like bind, connect send etc.) into member functions
 * which sometimes use a little more convenient parameter types (e.g. ArrayView instead of pointer+length)
 * It doesn't retain any state except the handle and may cache a few config flags.
 */
class Socket {

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
	bool is_valid() const noexcept { return _handle != port_layer::handle_t::Invalid; }

	ErrorCode close() noexcept
	{
		ErrorCode ret = port_layer::close_socket( _handle );
		if( ret ) { _handle = port_layer::handle_t::Invalid; }
		return ret;
	}

	port_layer::handle_t get_handle() const { return _handle; }

	port_layer::handle_t release() { return std::exchange( _handle, port_layer::handle_t::Invalid ); }

	/* ###### send / rec ############### */
	struct SendResult {
		mart::ConstMemoryView remaining_data;
		ErrorCode             result;
	};

	SendResult send( mart::ConstMemoryView data, int flags = 0 )
	{
		auto res = port_layer::send( _handle, _detail_socket_::to_byte_range( data ), flags );
		return {data.subview( res.value_or( 0 ) ), res.error_code()};
	}

	SendResult sendto( mart::ConstMemoryView data, int flags, const Sockaddr& addr )
	{
		auto res = port_layer::sendto( _handle, _detail_socket_::to_byte_range( data ), flags, addr );
		return {data.subview( res.value_or( 0 ) ), res.error_code()};
	}

	struct RecvResult {
		mart::MemoryView received_data;
		ErrorCode        result;
	};

	RecvResult recv( mart::MemoryView buffer, int flags )
	{
		auto ret = port_layer::recv( _handle, _detail_socket_::to_mutable_byte_range( buffer ), flags );
		if( ret.success() ) {
			return {buffer.subview( 0, ret.value() ), ret.error_code()};
		} else {
			return {mart::MemoryView {}, ret.error_code()};
		}
	}


	RecvResult recvfrom( mart::MemoryView buffer, int flags, Sockaddr& src_addr )
	{
		auto ret = port_layer::recvfrom( _handle, _detail_socket_::to_mutable_byte_range( buffer ), flags, src_addr );
		if( ret.success() ) {
			return {buffer.subview( 0, ret.value() ), ret.error_code()};
		} else {
			return {mart::MemoryView {}, ret.error_code()};
		}
	}

	/* ###### connection related ############### */

	auto bind( const Sockaddr& addr ) noexcept { return port_layer::bind( _handle, addr ); }

	auto connect( const Sockaddr& addr ) noexcept { return port_layer::connect( _handle, addr ); }

	auto listen( int backlog ) { return port_layer::listen( _handle, backlog ); }

	Socket accept()
	{
		auto res = port_layer::accept( _handle );
		if( res ) {
			return Socket( res.value() );
		} else {
			return Socket {};
		}
	}

	Socket accept( Sockaddr& remote_addr )
	{
		auto res = port_layer::accept( _handle, remote_addr );
		if( res ) {
			return Socket( res.value() );
		} else {
			return Socket {};
		}
	}

	/* ###### Configuration ############### */
	template<class T>
	ErrorCode setsockopt( SocketOptionLevel level, SocketOption optname, const T& option_data ) noexcept
	{
		auto opmem = byte_range_from_pod( option_data );
		return port_layer::setsockopt( _handle, level, optname, opmem );
	}

	template<class T>
	ErrorCode getsockopt( SocketOptionLevel level, SocketOption optname, T& option_data ) noexcept
	{
		return port_layer::getsockopt( _handle, level, optname, byte_range_from_pod( option_data ) );
	}

	ErrorCode set_blocking( bool should_block ) noexcept
	{
		if( _is_blocking == should_block ) { return {ErrorCodeValues::NoError}; }
		return _setBlocking_uncached( should_block );
	}

	bool is_blocking() const
	{
		// XXX: acutally query the native socket
		return _is_blocking && is_valid();
	}

	bool set_tx_timeout( std::chrono::microseconds timeout ) noexcept
	{
		return port_layer::set_timeout( _handle, Direction::Tx, timeout ).success();
	}

	bool set_rx_timeout( std::chrono::microseconds timeout ) noexcept
	{
		return port_layer::set_timeout( _handle, Direction::Rx, timeout ).success();
	}
	std::chrono::microseconds get_tx_timeout() noexcept
	{
		return port_layer::get_timeout( _handle, Direction::Tx ).value_or( {} );
	}

	std::chrono::microseconds get_rx_timeout() noexcept
	{
		return port_layer::get_timeout( _handle, Direction::Rx ).value_or( {} );
	}

private:
	ErrorCode _open( Domain domain, TransportType type )
	{
		auto res = port_layer::socket( domain, type );
		if( res ) {
			_handle = res.value();
			_setBlocking_uncached( true );
		}
		return res.error_code();
	}

	ErrorCode _setBlocking_uncached( bool should_block )
	{
		const auto res = port_layer::set_blocking( _handle, should_block );
		if( res ) {
			_is_blocking = should_block;
		}
		return res;
	}

	port_layer::handle_t _handle      = port_layer::handle_t::Invalid;
	bool                 _is_blocking = true;
};

} // namespace socks
} // namespace nw
} // namespace mart

#endif
