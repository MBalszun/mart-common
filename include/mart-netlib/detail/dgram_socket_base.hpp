#ifndef LIB_MART_COMMON_GUARD_NW_DETAIL_DGRAM_SOCKET_BASE_H
#define LIB_MART_COMMON_GUARD_NW_DETAIL_DGRAM_SOCKET_BASE_H

/**
 * dgram_socket_base.hpp (mart-common/nw/detail)
 *
 * Copyright (C) 2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	BAse class for data gram sockets
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
#include <mart-netlib/Socket.hpp>
#include <mart-netlib/port_layer.hpp>

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>
#include <mart-common/utils.h>

/* Standard Library Includes */
#include <chrono>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {
namespace socks {
namespace detail {

/*
* DgramSocketBase and the template DgramSocket provide the implementation for datagram based sockets
* such as udp (the udp socket class is just a specialization of DgramSocket for udp endpoints).
*
* Most of the implementation can be found in src/mart-netlib/detaildgram_socket_base_impl.hpp
* The goal is to reduce compile times for files that use default socket types like
* udp or unix domain sockets, but if you want to specialize DgramSocket for a differen
* endpoint type, you need to also include that file.
*
* Note, that instantiating for DgramSocket arbitrary user types is not supported by this library
* and will likely fail.
*/
class DgramSocketBase {
public:
	DgramSocketBase(mart::nw::socks::Domain domain);

	DgramSocketBase( DgramSocketBase&& ) noexcept = default;
	DgramSocketBase& operator=( DgramSocketBase&& ) noexcept = default;

	const nw::socks::RaiiSocket& getRawSocket() const { return _socket_handle; }
	nw::socks::RaiiSocket& getRawSocket() { return _socket_handle; }

	/* All send functions return the remaining (non-sent) data */
	auto try_send( mart::ConstMemoryView data ) noexcept -> mart::ConstMemoryView
	{
		return _socket_handle.send( data, 0 ).remaining_data;
	}
	auto send( mart::ConstMemoryView data ) -> mart::ConstMemoryView;

	mart::MemoryView try_recv( mart::MemoryView buffer ) noexcept
	{
		return _socket_handle.recv( buffer, 0 ).received_data;
	}
	mart::MemoryView recv( mart::MemoryView buffer );

	void clearRxBuff();

	bool set_tx_timeout( std::chrono::microseconds timeout ) { return _socket_handle.set_tx_timeout( timeout ); }
	bool set_rx_timeout( std::chrono::microseconds timeout ) { return _socket_handle.set_rx_timeout( timeout ); }
	std::chrono::microseconds get_tx_timeout() { return _socket_handle.get_tx_timeout(); }
	std::chrono::microseconds get_rx_timeout() { return _socket_handle.get_rx_timeout(); }

	bool set_blocking( bool should_block ) { return _socket_handle.set_blocking( should_block ).success(); }
	bool is_blocking() const { return _socket_handle.is_blocking(); }
	bool is_valid() const { return _socket_handle.is_valid(); }

	bool close() { return _socket_handle.close().success(); }

protected:
	nw::socks::RaiiSocket _socket_handle;
};

template<class EndpointT>
class DgramSocket : DgramSocketBase {
public:
	using endpoint = EndpointT;

	DgramSocket()
		: DgramSocketBase( mart::nw::socks::Domain::Local )
	{
		//assert( _socket_handle.is_valid() );
	};
	DgramSocket( const endpoint& local, const endpoint& remote );
	DgramSocket( DgramSocket&& ) noexcept = default;
	DgramSocket& operator=( DgramSocket&& ) noexcept = default;

	socks::ErrorCode try_bind( endpoint ep ) noexcept;
	void             bind( endpoint ep );

	socks::ErrorCode try_connect( endpoint ep ) noexcept;
	void             connect( endpoint ep );

	/* All send functions return the remaining (non-sent) data */
	auto try_sendto( mart::ConstMemoryView data, endpoint ep ) noexcept -> mart::ConstMemoryView
	{
		return _socket_handle.sendto( data, 0, ep.toSockAddr() ).remaining_data;
	}
	auto sendto( mart::ConstMemoryView data, endpoint ep ) -> mart::ConstMemoryView;

	struct RecvfromResult {
		mart::MemoryView data;
		endpoint         remote_address;
	};
	RecvfromResult try_recvfrom( mart::MemoryView buffer ) noexcept
	{
		using abi_endpoint_type = typename EndpointT::abi_endpoint_type;
		abi_endpoint_type addr {};
		auto              res = _socket_handle.recvfrom( buffer, 0, addr );
		return {res.received_data, endpoint( addr )};
	}
	RecvfromResult recvfrom( mart::MemoryView buffer );

	const endpoint& getLocalEndpoint() const { return _ep_local; }
	const endpoint& getRemoteEndpoint() const { return _ep_remote; }

private:
	endpoint _ep_local {};
	endpoint _ep_remote {};
	// SockaddrIn _sa_remote{}; //this is only for caching, so we don't have to convert _ep_remote to SockaddrIn every
	// time.
	nw::socks::RaiiSocket _socket_handle;
};



} // namespace detail
} // namespace socks
} // namespace nw
} // namespace mart

#endif