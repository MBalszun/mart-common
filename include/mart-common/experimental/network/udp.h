#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_UDP_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_UDP_H
/**
 * udp.h (mart-common/experimental/nw)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
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
/* Standard Library Includes */
#include <chrono>
#include <cerrno>
#include <cstring>

/* Proprietary Library Includes */
#include "../../utils.h"
#include "../../MartTime.h"
#include "../../ArrayView.h"

/* Project Includes */
#include "ip.h"
#include "Socket.h"
#include <im_str/im_str.hpp>
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace experimental {
namespace nw {
namespace ip {
namespace udp {

using endpoint = ip::_impl_details_ip::basic_endpoint_v4<TransportProtocol::UDP>;

class Socket {
public:
	Socket() :
		_socket_handle(socks::Domain::inet, socks::TransportType::datagram, 0)
	{}
	Socket(endpoint local, endpoint remote)
		: Socket()
	{
		if (!isValid()) {
			throw std::runtime_error(mba::concat("Could not create udp socket | Errnor:", std::to_string(errno), " msg: ", std::string_view(std::strerror(errno))).c_str());
		}
		if (!bind(local)) {
			throw std::runtime_error(mba::concat("Could not bind udp socket to address ", local.toStringEx(), "| Errnor:", std::to_string(errno), " msg: ", std::string_view(std::strerror(errno))).c_str());
		}
		if (!connect(remote)) {
			throw std::runtime_error(mba::concat("Could not connect socket to address ", local.toStringEx(), "| Errnor:", std::to_string(errno), " msg: ", std::string_view(std::strerror(errno))).c_str());
		}
	}


	Socket(Socket&&) = default;
	Socket& operator=(Socket&&) = default;

	const nw::socks::Socket& getSocket() const
	{
		return _socket_handle;
	}

	nw::socks::Socket& getSocket()
	{
		return _socket_handle;
	}

	void setDefaultRemoteEndpoint(endpoint ep)
	{
		_ep_remote = ep;
		_sa_remote = ep.toSockAddr_in();
	}
	bool connect(endpoint ep)
	{
		setDefaultRemoteEndpoint(ep);
		return _socket_handle.connect(_sa_remote) == 0;
	}
	bool bind(endpoint ep)
	{
		_ep_local = ep;
		auto result = _socket_handle.bind(ep.toSockAddr_in());
		return result == 0;
	}
	bool send(mart::ConstMemoryView data)
	{
		return _txWasSuccess(data, _socket_handle.send(data, 0));
	}
	bool sendto(mart::ConstMemoryView data, endpoint ep)
	{
		return _txWasSuccess(data, _socket_handle.sendto(data, 0, ep.toSockAddr_in()));
	}
	bool sendto(mart::ConstMemoryView data)
	{
		return _txWasSuccess(data, _socket_handle.sendto(data, 0, _sa_remote));
	}
	mart::MemoryView rec(mart::MemoryView buffer)
	{
		return _socket_handle.recv(buffer, 0).first;
	}
	mart::MemoryView recvfrom(mart::MemoryView buffer, endpoint& src_addr)
	{
		sockaddr_in src{};
		auto tmp = _socket_handle.recvfrom(buffer, 0, src);
		if (tmp.first.isValid() && src.sin_family == mart::toUType(nw::socks::Domain::inet)) {
			src_addr = endpoint(src);
			return tmp.first;
		} else {
			return mart::MemoryView{};
		}
	}
	void clearRxBuff()
	{
		//XXX: Make a RAII class for preserving the blocking state of the socket
		auto t = _socket_handle.isBlocking();
		_socket_handle.setBlocking(false);
		uint64_t _tmp{};
		auto tmp = mart::view_bytes_mutable(_tmp);
		try {
			while (_socket_handle.recv(tmp, 0).first.isValid()) { ; }
		} catch (...) {
			try {
				_socket_handle.setBlocking(t);
			} catch(...) {}
			throw;
		}
		_socket_handle.setBlocking(t);

	}
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
	sockaddr_in _sa_remote{}; //this is only for caching, so we don't have to convert _ep_remote to sockaddr_in  every time.
	nw::socks::Socket _socket_handle;
};

inline std::optional<endpoint> parse_v4_endpoint(std::string_view str) {
	return ip::_impl_details_ip::parse_v4_endpoint<ip::TransportProtocol::UDP>(str);
}

}//ns udp

}//ns ip
}//ns nw
}//ns exp
}//ns mart


#endif