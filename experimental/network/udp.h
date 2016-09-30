/*
* udp.h
*
*  Created on: 2016-09-29
*      Author: Michael Balszun <michael.balszun@tum.de>
*
*      This file provides a simple udp socket implementation
*/

#ifndef LIBS_MART_COMMON_EXPERIMENTAL_NW_UDP_H_
#define LIBS_MART_COMMON_EXPERIMENTAL_NW_UDP_H_
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <chrono>
#include <iostream> //TODO: remove

/* Proprietary Library Includes */
#include "../../utils.h"
#include "../../MartTime.h"
#include "../../ArrayView.h"

/* Project Includes */
#include "ip.h"
#include "NativeSocketWrapper.h"

namespace mart {
namespace experimental {
namespace nw {
namespace ip {
namespace udp {

using endpoint = ip::_impl_details_ip::basic_endpoint_v4<TransportProtocol::UDP>;

class Socket {
public:
	Socket() :
		_socket_handle(SocketDomain::inet, SocketType::datagram, 0)
	{}
	Socket(Socket&&) = default;
	Socket& operator=(Socket&&) = default;

	nw::NativeSocketWrapper& getSocket()
	{
		return _socket_handle;
	}
	void setDefaultRemoteEndpoint(endpoint ep)
	{
		_ep_remote = ep;
		_sa_remote = ep.toSockAddr_in();
	}
	void connect(endpoint ep)
	{
		setDefaultRemoteEndpoint(ep);
		_socket_handle.connect(_sa_remote);
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
		return _socket_handle.recv(buffer, 0).second;
	}
	mart::MemoryView recvfrom(mart::MemoryView buffer, endpoint& src_addr)
	{
		sockaddr_in src{};
		auto tmp = _socket_handle.recvfrom(buffer, 0, src);
		if (tmp.second.isValid() && src.sin_family == mart::toUType(nw::SocketDomain::inet)) {
			src_addr = endpoint(src);
			return tmp.second;
		} else {
			return mart::MemoryView{};
		}
	}
	void clearRxBuff()
	{
		auto t = _socket_handle.isBlocking();
		_socket_handle.setBlocking(false);
		uint64_t _tmp{};
		auto tmp = mart::viewMemory(_tmp);
		while (_socket_handle.recv(tmp, 0).first != EWOULDBLOCK) { ; }
		_socket_handle.setBlocking(t);
	}
	void setTxTimeout(std::chrono::microseconds timeout)
	{
		auto to = nw::to_timeval(timeout);
		_socket_handle.setsockopt(SOL_SOCKET, SO_SNDTIMEO, to);
	}
	void setRxTimeout(std::chrono::microseconds timeout)
	{
		auto to = nw::to_timeval(timeout);
		_socket_handle.setsockopt(SOL_SOCKET, SO_RCVTIMEO, to);
	}
	std::chrono::microseconds getTxTimeout()
	{
		timeval to;
		_socket_handle.getsockopt(SOL_SOCKET, SO_SNDTIMEO, to);
		return nw::from_timeval<std::chrono::microseconds>(to);
	}
	std::chrono::microseconds getRxTimeout()
	{
		timeval to;
		_socket_handle.getsockopt(SOL_SOCKET, SO_RCVTIMEO, to);
		return nw::from_timeval<std::chrono::microseconds>(to);
	}
	bool setBlocking(bool should_block)
	{
		return _socket_handle.setBlocking(should_block);
	}
	bool isValid() const
	{
		return _socket_handle.isValid();
	}
	const endpoint& getLocalEndpoint() { return _ep_local; }
	const endpoint& getRemoteEndpoint() { return _ep_remote; }
private:
	bool _txWasSuccess(mart::ConstMemoryView data, nw::port::sock::txrx_size_type ret)
	{
		return mart::narrow<nw::port::sock::txrx_size_type>(data.size_inBytes()) == ret;
	}
	endpoint _ep_local{};
	endpoint _ep_remote{};
	sockaddr_in _sa_remote{}; //this is only for caching, so we don't have to convert _ep_remote to sockaddr_in  every time.
	nw::NativeSocketWrapper _socket_handle;
};

}//ns udp

}//ns ip
}//ns nw
}//ns exp
}//ns mart


#endif