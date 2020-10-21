#ifndef LIB_MART_COMMON_GUARD_NW_TCP_HPP
#define LIB_MART_COMMON_GUARD_NW_TCP_HPP
/**
 * udp.h (mart-netlib)
 *
 * Copyright (C) 2020 Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	This file provides a simple udp socket implementation
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
#include "RaiiSocket.hpp"
#include "ip.hpp"

#include "detail/socket_base.hpp"

#include <mart-netlib/network_exceptions.hpp>

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>

/* Standard Library Includes */
#include <cassert>
#include <chrono>
#include <optional>
#include <string_view>

#if __has_include( <charconv> )
#include <charconv>
#endif

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {
namespace ip {
namespace tcp {

inline std::string_view errno_nr_as_string( mart::nw::socks::ErrorCode error, mart::ArrayView<char> buffer )
{
#if __has_include( <charconv> )
	auto res = std::to_chars( buffer.begin(), buffer.end(), error.raw_value() );
	return { buffer.begin(), static_cast<std::string_view::size_type>( res.ptr - buffer.begin() ) };
#else
	auto res = std::to_string( error.raw_value() );
	auto n   = std::min( res.size(), buffer.size() );

	std::copy_n( res.begin(), n, buffer.begin() );
	return { buffer.begin(), n };
#endif
}

inline std::string_view errno_nr_as_string( mart::ArrayView<char> buffer )
{
	return errno_nr_as_string( mart::nw::socks::port_layer::get_last_socket_error(), buffer );
}

template<class... Elements>
mba::im_zstr make_error_message_with_appended_last_errno( mart::nw::socks::ErrorCode error, Elements&&... elements )
{
	std::array<char, 24> errno_buffer{};
	return mba::concat( std::string_view( elements )...,
						"| Error Code:",
						errno_nr_as_string( error, errno_buffer ),
						" Error Msg: ",
						socks::to_text_rep( error ) );
}

using endpoint = ip::basic_endpoint_v4<mart::nw::ip::TransportProtocol::Tcp>;

class Acceptor;
class Socket : public mart::nw::socks::detail::HighLevelSocketBase {
public:
	Socket()
		: mart::nw::socks::detail::HighLevelSocketBase( socks::Domain::Inet, socks::TransportType::Stream )
	{
	}
	Socket( endpoint local, endpoint remote )
		: Socket()
	{
		bind( local );
		connect( remote );
		// TODO: throw exception, if bind fails
	}
	Socket( Socket&& other ) noexcept
		: mart::nw::socks::detail::HighLevelSocketBase( std::move( other ) )
		, _ep_local( std::move( other._ep_local ) )
		, _ep_remote( std::move( other._ep_remote ) )
	{
		other = Socket{};
	}
	Socket& operator=( Socket&& other ) noexcept
	{
		mart::nw::socks::detail::HighLevelSocketBase::operator=( std::move( other ) );
		_ep_local                                             = std::exchange( other._ep_local, endpoint{} );
		_ep_remote                                            = std::exchange( other._ep_remote, endpoint{} );
		return *this;
	}
	void connect( endpoint ep )
	{
		auto result = _socket.connect( ep.toSockAddr() );
		if( !result.success() ) {
			throw generic_nw_error( make_error_message_with_appended_last_errno(
				result, "Could not connect socket to address ", ep.toStringEx() ) );
		}
		_ep_remote = ep;

		auto t_ep = getSockAddress( _socket );
		if( !t_ep.result.success() ) {
			throw generic_nw_error(
				make_error_message_with_appended_last_errno( t_ep.result, "Could not get port of connected socket " ) );
		}
		_ep_local = t_ep.ep;
	}

	bool try_connect( endpoint ep )
	{
		auto result = _socket.connect( ep.toSockAddr() );
		if( !result.success() ) { return false; }
		_ep_remote = ep;

		auto t_ep = getSockAddress( _socket );
		if( !t_ep.result.success() ) { return false; }
		_ep_local = t_ep.ep;
		return true;
	}

	void bind( endpoint ep )
	{
		assert( _socket.is_valid() );
		auto result = _socket.bind( ep.toSockAddr_in() );
		if( !result.success() ) {
			throw generic_nw_error( make_error_message_with_appended_last_errno(
				result, "Could not bind tcp socket to address ", ep.toStringEx() ) );
		}

		_ep_local = ep;
	}
	void send( mart::ConstMemoryView data )
	{
		while( !data.empty() ) {
			const auto res = _socket.send( data, 0 );
			if( !_txWasSuccess( data, res ) ) {
				throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
					res.result.error_code(), "Failed to send data. Details:  " ) );
			}
			data = res.remaining_data;
		}
	}

	template<class T, T... Vals>
	bool is_none_of( T v )
	{
		return ( true && ... && ( v != Vals ) );
	}

	mart::MemoryView recv( mart::MemoryView buffer )
	{
		using mart::nw::socks::ErrorCodeValues;
		const auto res = _socket.recv( buffer, 0 );
		if( !res.result
			&& is_none_of<ErrorCodeValues,
						  ErrorCodeValues::WouldBlock,
						  ErrorCodeValues::TryAgain,
						  ErrorCodeValues::Timeout>( res.result.error_code().value() ) ) {
			throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
				res.result.error_code(), "Failed to receive data. Details:  " ) );
		}
		return res.received_data;
	}

	const endpoint& get_local_endpoint() const { return _ep_local; }
	const endpoint& get_remote_endpoint() const { return _ep_remote; }

private:
	struct RetForGetSockAddress {
		mart::net::socks::ErrorCode result;
		endpoint                    ep;
	};
	static RetForGetSockAddress getSockAddress( const nw::socks::RaiiSocket& socket )
	{
		RetForGetSockAddress ret;

		mart::net::socks::port_layer::SockaddrIn t_locaddr{};
		ret.result = socket.getsockname( t_locaddr );

		if( ret.result ) {
			ret.ep = endpoint( t_locaddr );
		} else {
			ret.ep = endpoint{};
		}

		return ret;
	}

	static inline bool _txWasSuccess( mart::ConstMemoryView data, const mart::nw::socks::RaiiSocket::SendResult& ret )
	{
		return ret.result.success() && mart::narrow<nw::socks::txrx_size_t>( data.size() ) == ret.result.value();
	}

	Socket( net::socks::RaiiSocket&& sock, endpoint local, endpoint remote )
		: mart::nw::socks::detail::HighLevelSocketBase( std::move( sock ) )
		, _ep_local( local )
		, _ep_remote( remote )
	{
	}

	friend Acceptor;
	endpoint _ep_local{};
	endpoint _ep_remote{};
};

inline Socket connect( endpoint ep )
{
	Socket s;
	s.connect( ep );
	return s;
}

class Acceptor {
	enum class State { open, bound, listening };

public:
	Acceptor()
		: _socket_handle( socks::Domain::Inet, socks::TransportType::Stream )
	{
		if( !_socket_handle.is_valid() ) {
			// TODO: The creation of this exception message seems to be pretty costly in terms of binary size - have to
			// investigate NOTE: Preliminary tests suggest, that the dynamic memory allocation for the message text
			// could be the main problem, but that is just a possibility
			throw generic_nw_error( make_error_message_with_appended_last_errno(
				mart::nw::socks::port_layer::get_last_socket_error(), "Could not create tcp acceptor." ) );
		}
	}

	Acceptor( Acceptor&& other ) noexcept
		: _socket_handle( std::move( other._socket_handle ) )
		, _ep_local( std::move( other._ep_local ) )
		, _state( other._state )
	{
		other = Acceptor{};
	}
	Acceptor& operator=( Acceptor&& other ) noexcept
	{
		_socket_handle  = std::move( other._socket_handle );
		_ep_local       = std::move( other._ep_local );
		other._ep_local = endpoint{};
		return *this;
	}

	// will create acceptor bound to endpoint and listening for incomming connections
	Acceptor( endpoint local, int backlog = 10 )
		: Acceptor::Acceptor()
	{
		bind( local );
		listen( backlog );
	}

	/* Binds to an address and immediately starts to listen on that address*/
	void bind( endpoint ep )
	{
		assert( _state == State::open );

		auto result = _socket_handle.bind( ep.toSockAddr_in() );
		if( !result.success() ) {
			throw generic_nw_error( make_error_message_with_appended_last_errno(
				result, "Could not bind tcp acceptor to address ", ep.toStringEx() ) );
		}

		_ep_local = ep;
		_state    = State::bound;
	}

	bool try_bind( endpoint ep )
	{
		assert( _state == State::open );

		auto result = _socket_handle.bind( ep.toSockAddr_in() );
		if( !result.success() ) { return false; }

		_ep_local = ep;
		_state    = State::bound;
		return true;
	}

	void listen( int backlog = 10 )
	{
		assert( _state == State::bound );

		auto result = _socket_handle.listen( backlog );
		if( !result.success() ) {
			throw generic_nw_error( make_error_message_with_appended_last_errno(
				result, "Tcp acceptor could not start to listen on address ", _ep_local.toStringEx() ) );
		}
		_state = State::listening;
	}

	bool try_listen( int backlog = 10 )
	{
		assert( _state == State::bound );

		auto result = _socket_handle.listen( backlog );
		if( !result.success() ) { return false; }
		_state = State::listening;
		return true;
	}

	bool is_valid() { return _socket_handle.is_valid(); }

public:
	Socket try_accept()
	{
		assert( _state == State::listening );
		_socket_handle.set_blocking( false );

		mart::net::socks::port_layer::SockaddrIn addr;

		auto sock = _socket_handle.accept( addr );
		auto res  = Socket::getSockAddress( sock );
		if( sock.is_valid() && res.result.success() ) {
			return Socket( std::move( sock ), res.ep, endpoint( addr ) );
		} else {
			return {};
		}
	}

	Socket accept( std::chrono::microseconds timeout = std::chrono::hours( 300 ) )
	{
		_socket_handle.set_blocking( true );
		_socket_handle.set_rx_timeout( timeout );
		_socket_handle.set_tx_timeout( timeout );

		mart::net::socks::port_layer::SockaddrIn addr;

		auto sock = _socket_handle.accept( addr );
		auto res  = Socket::getSockAddress( sock );
		if( sock.is_valid() && res.result.success() ) {
			return Socket( std::move( sock ), res.ep, endpoint( addr ) );
		} else {
			return {};
		}
	}

	nw::socks::RaiiSocket& getSocket() { return _socket_handle; }

	endpoint getLocalEndpoint() const { return _ep_local; }

private:
	nw::socks::RaiiSocket _socket_handle;
	endpoint              _ep_local{};
	State                 _state = State::open;
};

inline std::optional<endpoint> parse_v4_endpoint( std::string_view str )
{
	return ip::try_parse_v4_endpoint<ip::TransportProtocol::Tcp>( str );
}

} // namespace tcp

} // namespace ip
} // namespace nw
} // namespace mart

#endif