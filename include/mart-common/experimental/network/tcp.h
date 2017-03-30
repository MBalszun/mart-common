#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_TCP_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_TCP_H
/*
* tcp.h
*
*  Created on: 2016-09-29
*      Author: Michael Balszun <michael.balszun@tum.de>
*
*      This file provides a simple tcp socket implementation
*/


/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <chrono>
#include <iostream>

/* Proprietary Library Includes */
#include "../../utils.h"
#include "../../MartTime.h"
#include "../../ArrayView.h"

/* Project Includes */
#include "ip.h"
#include "Socket.h"

namespace mart {
namespace experimental {
namespace nw {
namespace ip {
namespace tcp {

using endpoint = ip::_impl_details_ip::basic_endpoint_v4<TransportProtocol::TCP>;

class Acceptor;
class Socket {
public:
	Socket() :
		_socket_handle(socks::Domain::inet, socks::TransportType::stream, 0)
	{}
	Socket(endpoint ep) :
		Socket()
	{
		bind(ep);
		//TODO: throw exception, if bind fails
	}
	Socket(Socket&& other) :
		_socket_handle(std::move(other._socket_handle)),
		_ep_local(std::move(other._ep_local)),
		_ep_remote(std::move(other._ep_remote))
	{
		other = Socket{};
	}
	Socket& operator=(Socket&& other)
	{
		_socket_handle = std::move(other._socket_handle);
		_ep_local = mart::exchange(other._ep_local,endpoint{});
		_ep_remote = mart::exchange(other._ep_remote, endpoint{});
		return *this;
	}
	bool connect(endpoint ep)
	{
		_ep_remote = ep;
		auto t = _socket_handle.connect(ep.toSockAddr_in());
		auto t_ep = getSockAddress(_socket_handle);
		if (t_ep.valid) {
			_ep_local = t_ep;
		}
		return t == 0;
	}
	bool bind(endpoint ep)
	{
		_ep_local = ep;
		return _socket_handle.bind(ep.toSockAddr_in()) == 0;
	}
	mart::ConstMemoryView send(mart::ConstMemoryView data)
	{
		auto ret = _socket_handle.send(data, 0);
		if (ret >= 0) {
			return data.subview(ret);
		} else {
			return mart::ConstMemoryView();
		}
	}
	mart::MemoryView recv(mart::MemoryView buffer)
	{
		return _socket_handle.recv(buffer, 0).first;
	}
	nw::socks::Socket& getSocket()
	{
		return _socket_handle;
	}
	const nw::socks::Socket& getSocket() const
	{
		return _socket_handle;
	}
	void setTxTimeout(std::chrono::microseconds timeout)
	{
		_socket_handle.setTxTimeout(timeout);
	}
	void setRxTimeout(std::chrono::microseconds timeout)
	{
		_socket_handle.setRxTimeout(timeout);
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
	static endpoint getSockAddress(const nw::socks::Socket& socket)
	{
		sockaddr_in t_locaddr{};
		socklen_t addrlenLocal = sizeof(t_locaddr);
		if (getsockname(socket.getNative(), (sockaddr *)&t_locaddr, &addrlenLocal) == -1) {
			return endpoint{};
		} else {
			return endpoint(t_locaddr);
		}
	}


	friend Acceptor;
	nw::socks::Socket _socket_handle;
	endpoint _ep_local{};
	endpoint _ep_remote{};
};

class Acceptor {
	enum class State {
		closed,
		open,
		bound,
		listening
	};
public:
	Acceptor() = default;
	//will create acceptor bound to endpoint and listening for incomming connections
	Acceptor(endpoint ep) :
		Acceptor()
	{
		listen_on(ep);
		//TODO: throw exception, if this fails
	}
	Acceptor(Acceptor&& other) :
		_socket_handle(std::move(other._socket_handle)),
		_ep_local(std::move(other._ep_local)),
		_state(other._state)
	{
		other = Acceptor{};
	}
	Acceptor& operator=(Acceptor&& other)
	{
		_socket_handle = std::move(other._socket_handle);
		_ep_local = std::move(other._ep_local);
		other._ep_local = endpoint{};
		return *this;
	}
	/* Binds to an address and immediately starts to listen on that address*/
	bool open()
	{
		_socket_handle = nw::socks::Socket(socks::Domain::inet, socks::TransportType::stream, 0);
		if (_socket_handle.isValid()) {
			_state = State::open;
			return true;
		} else {
			_state = State::closed;
			return false;
		}
	}
	bool bind(endpoint ep)
	{
		if (_state != State::open) {
			return false;
		}
		if (_socket_handle.bind(ep.toSockAddr_in()) == 0) {
			_ep_local = ep;
			_state = State::bound;
			return true;
		} else {
			return false;
		}
	}
	bool listen( int backlog = 10)
	{
		if (_state != State::bound) {
			return false;
		}
		if( _socket_handle.listen(backlog) == 0) {
			_state = State::listening;
			return true;
		} else {
			return false;
		}
	}

	bool listen_on(endpoint ep, int backlog = 10)
	{
		switch(_state ) {
			case State::listening : {
				_socket_handle = nw::socks::Socket();
				this->_ep_local = tcp::endpoint{};
				this->_state = State::closed;
			}//deliberate fall through
			case State::closed: {
				open();
			}//deliberate fall through
			case State::open: {
				bind(ep);
			}//deliberate fall through
			case State::bound : {
				return listen(backlog);
			}
		}
		assert(false);
		return false; //should not reach here
	}


	bool isValid()
	{
		return _socket_handle.isValid();
	}
	Socket accept(std::chrono::microseconds timeout = std::chrono::hours(300))
	{
		_socket_handle.setBlocking(true);
		_socket_handle.setRxTimeout(timeout);
		sockaddr_in sa_remote{};
		auto handle = _socket_handle.accept(sa_remote);
		if (!handle.isValid()) {
			return Socket{};
		}

		Socket ret{};
		ret._socket_handle = std::move(handle);
		ret._ep_local = _ep_local;
		ret._ep_remote = endpoint(sa_remote);
		return ret;
	}
	nw::socks::Socket& getSocket()
	{
		return _socket_handle;
	}

	endpoint getLocalEndpoint() const { return _ep_local; }
private:
	nw::socks::Socket _socket_handle;
	endpoint _ep_local{};
	State _state = State::closed;
};

}//ns udp

}//ns ip
}//ns nw
}//experimental
}//ns mart


#endif