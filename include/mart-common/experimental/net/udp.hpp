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
#include "ip.hpp"
#include "Socket.hpp"

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>

/* Standard Library Includes */
#include <chrono>
#include <optional>
#include <string_view>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {
namespace ip {
namespace udp {

using endpoint = ip::_impl_details_ip::basic_endpoint_v4<mart::nw::ip::TransportProtocol::UDP>;

class Socket {
public:
	Socket();
	Socket( endpoint local, endpoint remote );
	Socket(Socket&&) noexcept = default;
	Socket& operator=( Socket&& ) noexcept = default;

	const nw::socks::Socket& getRawSocket() const
	{
		return _socket_handle;
	}

	nw::socks::Socket& getRawSocket()
	{
		return _socket_handle;
	}

	void bind( endpoint ep );
	bool try_bind( endpoint ep ) noexcept;
	void connect( endpoint ep );
	bool try_connect( endpoint ep ) noexcept;

	bool send( mart::ConstMemoryView data )
	{
		return _txWasSuccess( data, _socket_handle.send( data, 0 ) );
	}

	mart::MemoryView rec(mart::MemoryView buffer)
	{
		return _socket_handle.recv(buffer, 0).first;
	}

	bool sendto( mart::ConstMemoryView data, endpoint ep ) noexcept;
	mart::MemoryView recvfrom( mart::MemoryView buffer, endpoint& src_addr ) noexcept;

	void clearRxBuff();

	bool setTxTimeout(std::chrono::microseconds timeout)
	{
		return _socket_handle.setTxTimeout(timeout);
	}
	bool setRxTimeout(std::chrono::microseconds timeout)
	{
		return _socket_handle.setRxTimeout(timeout);
	}
	std::chrono::microseconds getTxTimeout()
	{
		return _socket_handle.getTxTimeout();
	}
	std::chrono::microseconds getRxTimeout()
	{
		return _socket_handle.getRxTimeout();
	}
	bool setBlocking(bool should_block)
	{
		return _socket_handle.setBlocking(should_block);
	}
	bool isBlocking()
	{
		return _socket_handle.isBlocking();
	}
	bool isValid() const
	{
		return _socket_handle.isValid();
	}

	bool close()
	{
		return _socket_handle.close();
	}

	const endpoint& getLocalEndpoint() const { return _ep_local; }
	const endpoint& getRemoteEndpoint() const { return _ep_remote; }
private:
	bool _txWasSuccess(mart::ConstMemoryView data, nw::socks::port_layer::txrx_size_t ret)
	{
		return mart::narrow<nw::socks::port_layer::txrx_size_t>(data.size()) == ret;
	}
	endpoint _ep_local{};
	endpoint _ep_remote{};
	//sockaddr_in _sa_remote{}; //this is only for caching, so we don't have to convert _ep_remote to sockaddr_in  every time.
	nw::socks::Socket _socket_handle;
};

constexpr std::optional<endpoint> parse_v4_endpoint(std::string_view str) {
	return ip::parse_v4_endpoint<ip::TransportProtocol::UDP>(str);
}

}//ns udp

}//ns ip
}//ns nw
}//ns mart


#endif