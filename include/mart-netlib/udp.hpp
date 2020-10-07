#ifndef LIB_MART_COMMON_GUARD_NW_UDP_H
#define LIB_MART_COMMON_GUARD_NW_UDP_H
/**
 * udp.h (mart-common/nw)
 *
 * Copyright (C) 2015-2019: Michael Balszun <michael.balszun@mytum.de>
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
#include "Socket.hpp"
#include "ip.hpp"

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>

/* Standard Library Includes */
#include <cassert>
#include <chrono>
#include <optional>
#include <string_view>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {
namespace ip {
namespace udp {

using endpoint = ip::basic_endpoint_v4<mart::nw::ip::TransportProtocol::Udp>;

/* WARNING: This is meant as a convenience class around the generic RaiiSocket for udp communication. Its interface is
 * still very much in flux */
class Socket {
public:
	Socket();
	Socket( endpoint local, endpoint remote );
	Socket( Socket&& ) noexcept = default;
	Socket& operator=( Socket&& ) noexcept = default;

	const nw::socks::RaiiSocket& getRawSocket() const { return _socket_handle; }

	nw::socks::RaiiSocket& getRawSocket() { return _socket_handle; }

	socks::ErrorCode try_bind( endpoint ep ) noexcept;
	void             bind( endpoint ep );

	socks::ErrorCode try_connect( endpoint ep ) noexcept;
	void             connect( endpoint ep );

	bool try_send( mart::ConstMemoryView data ) noexcept
	{
		auto ret = _socket_handle.send( data, 0 );
		return _txWasSuccess(data, ret);
	}
	void send( mart::ConstMemoryView data );

	bool try_sendto( mart::ConstMemoryView data, endpoint ep ) noexcept
	{
		auto ret = _socket_handle.sendto( data, 0, ep.toSockAddr_in() );
		return _txWasSuccess( data, ret );
	}
	bool try_sendto_default( mart::ConstMemoryView data )
	{
		assert( _ep_remote.valid );
		auto ret = _socket_handle.sendto( data, 0, _ep_remote.toSockAddr_in() );
		return _txWasSuccess(data,ret);
	}
	void sendto( mart::ConstMemoryView data, endpoint ep );
	void sendto_default( mart::ConstMemoryView data )
	{
		assert( _ep_remote.valid );
		sendto( data, _ep_remote );
	}

	mart::MemoryView try_recv( mart::MemoryView buffer ) noexcept
	{
		return _socket_handle.recv( buffer, 0 ).received_data;
	}
	mart::MemoryView recv( mart::MemoryView buffer );

	struct RecvfromResult {
		mart::MemoryView data;
		endpoint         remote_address;
	};
	RecvfromResult try_recvfrom( mart::MemoryView buffer ) noexcept
	{
		mart::nw::socks::port_layer::SockaddrIn addr{};
		auto                                    res = _socket_handle.recvfrom( buffer, 0, addr );
		return { res.received_data, endpoint( addr ) };
	}
	RecvfromResult recvfrom( mart::MemoryView buffer );

	void clearRxBuff();

	bool set_tx_timeout( std::chrono::microseconds timeout ) { return _socket_handle.set_tx_timeout( timeout ); }
	bool set_rx_timeout( std::chrono::microseconds timeout ) { return _socket_handle.set_rx_timeout( timeout ); }
	std::chrono::microseconds get_tx_timeout() const { return _socket_handle.get_tx_timeout(); }
	std::chrono::microseconds get_rx_timeout() const { return _socket_handle.get_rx_timeout(); }

	bool set_blocking( bool should_block ) { return _socket_handle.set_blocking( should_block ).success(); }
	bool is_blocking() { return _socket_handle.is_blocking(); }
	bool is_valid() const { return _socket_handle.is_valid(); }

	bool close()
	{
		_ep_local  = {};
		_ep_remote = {};
		return _socket_handle.close().success();
	}

	[[deprecated]] const endpoint& getLocalEndpoint() const noexcept { return _ep_local; }
	[[deprecated]] const endpoint& getRemoteEndpoint() const noexcept { return _ep_remote; }

	const endpoint& get_local_endpoint() const { return _ep_local; }
	const endpoint& get_remote_endpoint() const { return _ep_remote; }

	void set_default_remote_endpoint( endpoint ep ) noexcept { _ep_remote = std::move( ep ); }

private:
	static inline bool _txWasSuccess( mart::ConstMemoryView data, const mart::nw::socks::RaiiSocket::SendResult& ret )
	{
		return ret.result.success() && mart::narrow<nw::socks::txrx_size_t>( data.size() ) == ret.result.value();
	}
	endpoint _ep_local{};
	endpoint _ep_remote{};
	// SockaddrIn _sa_remote{}; //this is only for caching, so we don't have to convert _ep_remote to SockaddrIn every
	// time.
	nw::socks::RaiiSocket _socket_handle;
};

constexpr std::optional<endpoint> try_parse_v4_endpoint( std::string_view str ) noexcept
{
	return ip::try_parse_v4_endpoint<ip::TransportProtocol::Udp>( str );
}

} // namespace udp

} // namespace ip
} // namespace nw
} // namespace mart

#endif