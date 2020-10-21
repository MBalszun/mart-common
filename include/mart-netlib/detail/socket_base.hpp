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
#include <mart-netlib/RaiiSocket.hpp>
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

class HighLevelSocketBase {
public:
	HighLevelSocketBase( mart::nw::socks::Domain domain, mart::nw::socks::TransportType type )
		: _socket( domain, type )
	{
	}

	explicit HighLevelSocketBase( nw::socks::RaiiSocket&& socket )
		: _socket( std::move( socket ) )
	{
	}

	HighLevelSocketBase( HighLevelSocketBase&& ) noexcept = default;
	HighLevelSocketBase& operator=( HighLevelSocketBase&& ) noexcept = default;

	// clang-format off
	inline const nw::socks::RaiiSocket& as_raii_socket() const noexcept { return _socket; }
	inline       nw::socks::RaiiSocket& as_raii_socket()       noexcept { return _socket; }
	inline       port_layer::handle_t   get_raw_socket_handle() const { return _socket.get_handle(); }

	[[deprecated]] inline const nw::socks::RaiiSocket& getRawSocket() const noexcept { return _socket; }
	[[deprecated]] inline       nw::socks::RaiiSocket& getRawSocket()       noexcept { return _socket; }


	inline bool                      try_set_tx_timeout( std::chrono::microseconds timeout )       noexcept { return _socket.set_tx_timeout( timeout ); }
	inline bool                      try_set_rx_timeout( std::chrono::microseconds timeout )       noexcept { return _socket.set_rx_timeout( timeout ); }
	inline std::chrono::microseconds try_get_tx_timeout()                                    const noexcept { return _socket.get_tx_timeout(); }
	inline std::chrono::microseconds try_get_rx_timeout()                                    const noexcept { return _socket.get_rx_timeout(); }

	void                      set_tx_timeout( std::chrono::microseconds timeout );
	void                      set_rx_timeout( std::chrono::microseconds timeout );
	std::chrono::microseconds get_tx_timeout() const;
	std::chrono::microseconds get_rx_timeout() const;

	inline bool try_set_blocking( bool should_block )       noexcept { return _socket.set_blocking( should_block ).success(); }
	       void set_blocking( bool should_block );
	inline bool is_blocking()                         const noexcept { return _socket.is_blocking(); }
	inline bool is_valid()                            const noexcept { return _socket.is_valid(); }

	inline bool try_close() noexcept { return _socket.close().success(); }
	void close();

	// clang-format on
protected:
	nw::socks::RaiiSocket _socket;
};

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
class DgramSocketBase : public HighLevelSocketBase {
public:
	DgramSocketBase( mart::nw::socks::Domain domain );

	DgramSocketBase( DgramSocketBase&& ) noexcept = default;
	DgramSocketBase& operator=( DgramSocketBase&& ) noexcept = default;

	/* With datagrams, either the full message is sent, or we failed */
	bool try_send( mart::ConstMemoryView data ) noexcept
	{
		auto r = _socket.send( data, 0 );
		return r.result.success();
	}
	void send( mart::ConstMemoryView data );

	mart::MemoryView try_recv( mart::MemoryView buffer ) noexcept { return _socket.recv( buffer, 0 ).received_data; }
	mart::MemoryView recv( mart::MemoryView buffer );

	void clearRxBuff();
};

template<class EndpointT>
class DgramSocket : public DgramSocketBase {
public:
	using endpoint = EndpointT;

	DgramSocket()
		: DgramSocketBase( endpoint::domain ){
			// assert( _socket_handle.is_valid() );
		};

	DgramSocket( endpoint local, endpoint remote );
	DgramSocket( DgramSocket&& ) noexcept = default;
	DgramSocket& operator=( DgramSocket&& ) noexcept = default;

	socks::ErrorCode try_bind( endpoint ep ) noexcept;
	void             bind( endpoint ep );

	socks::ErrorCode try_connect( endpoint ep ) noexcept;
	void             connect( endpoint ep );

	bool try_sendto( mart::ConstMemoryView data, endpoint ep ) noexcept
	{
		auto ret = _socket.sendto( data, 0, ep.toSockAddr() );
		return _txWasSuccess( data, ret );
	}
	bool try_sendto_default( mart::ConstMemoryView data )
	{
		assert( _ep_remote.valid() );
		auto ret = _socket.sendto( data, 0, _ep_remote.toSockAddr() );
		return _txWasSuccess( data, ret );
	}
	void sendto( mart::ConstMemoryView data, endpoint ep );

	void sendto_default( mart::ConstMemoryView data )
	{
		assert( _ep_remote.valid() );
		sendto( data, _ep_remote );
	}

	struct RecvfromResult {
		mart::MemoryView data;
		endpoint         remote_address;
	};
	RecvfromResult try_recvfrom( mart::MemoryView buffer ) noexcept
	{
		using abiep = typename EndpointT::abi_endpoint_type;
		abiep addr{};

		auto res = _socket.recvfrom( buffer, 0, addr );

		return { res.received_data, endpoint( addr ) };
	}
	RecvfromResult recvfrom( mart::MemoryView buffer );

	void clearRxBuff();

	auto close()
	{

		_ep_local  = {};
		_ep_remote = {};
		return DgramSocketBase::close();
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
};

} // namespace detail
} // namespace socks
} // namespace nw
} // namespace mart

#endif