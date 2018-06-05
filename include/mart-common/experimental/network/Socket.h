#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_SOCKET_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_SOCKET_H
/**
 * Socket.h (mart-common/experimental/nw)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides mart::experimental::Socket - an RAII-wrapper around the OS-socket handle
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */

/* Proprietary Library Includes */
#include "../../cpp_std/utility.h"
#include "../../ArrayView.h"

/* Project Includes */
#include "basic_types.h"
#include "port_layer.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */


namespace mart {
namespace experimental {
namespace nw {

template<class Dur>
timeval to_timeval(Dur duration)
{
	using namespace std::chrono;
	timeval ret{};
	if (duration.count() > 0) {
		auto s = duration_cast<seconds>(duration);
		ret.tv_sec = mart::narrow<decltype(ret.tv_sec)>(s.count());
		ret.tv_usec = mart::narrow<decltype(ret.tv_usec)>((duration_cast<microseconds>(duration) - s).count());
	}
	return ret;
}

template<class Dur>
Dur from_timeval(timeval duration)
{
	using namespace std::chrono;
	return duration_cast<Dur>(seconds(duration.tv_sec) + microseconds(duration.tv_usec));
}

namespace socks {

namespace _impl_details_nw {

//on windows, the wa system has to be initialized before sockets can be used
inline bool startUp() {
	const static bool isInit = port_layer::waInit();
	return isInit;
}

} //_impl_details_nw

enum class Domain {
	local = AF_UNIX,
	inet = AF_INET,
	inet6 = AF_INET6,
};

enum class TransportType {
	stream = SOCK_STREAM,
	datagram = SOCK_DGRAM,
	seqpacket = SOCK_SEQPACKET,
};



template<Domain, TransportType>
struct socket_traits {};

template<TransportType Type>
struct socket_traits<Domain::local, Type> {
	using address_t = sockaddr_un;
};
template<TransportType Type>
struct socket_traits<Domain::inet, Type> {
	using address_t = sockaddr_in;
};

template<TransportType Type>
struct socket_traits<Domain::inet6, Type> {
	using address_t = sockaddr_in6;
};

template<class T>
struct is_sock_addr_type {
	static constexpr bool value =
		//std::is_same<T, sockaddr>::value ||
		std::is_same<T, sockaddr_in>::value ||
		std::is_same<T, sockaddr_un>::value ||
		std::is_same<T, sockaddr_in6>::value;
};

/* This class is a very thin RAII wrapper around the OS's native socket handle.
 * It also translates the native socket function (like bind, connect send etc.) into member functions
 * which sometimes use a little more convenient parameter types (e.g. ArrayView instead of pointer+length)
 * It doesn't retain any state except the handle
 */
class Socket {
	template<class T>
	static sockaddr* asSockAddrPtr(T& addr)
	{
		static_assert(is_sock_addr_type<T>::value, "addr is not a valid socked address type");
		return reinterpret_cast<sockaddr*>(&addr);
	}

	template<class T>
	static const sockaddr* asSockAddrPtr(const T& addr)
	{
		static_assert(is_sock_addr_type<T>::value, "addr is not a valid socked address type");
		return reinterpret_cast<const sockaddr*>(&addr);
	}
public:
	/*##### CTORS / DTORS #####*/
	constexpr Socket() = default;
	explicit Socket(port_layer::handle_t handle) :
		_handle(handle)
	{}
	Socket(Domain domain, TransportType type, int protocol = 0)
	{
		_open(domain, type, protocol);
	}
	~Socket()
	{
		close();
	}

	/*##### Special member functions #####*/
	Socket(const Socket& other) = delete;
	Socket& operator=(const Socket& other) = delete;

	Socket(Socket&& other) :
		_handle{ mart::exchange(other._handle, port_layer::invalid_handle) },
		_is_blocking{ mart::exchange(other._is_blocking, true) }
	{}

	Socket& operator=(Socket&& other)
	{
		close();
		_handle = mart::exchange(other._handle, port_layer::invalid_handle);
		_is_blocking = mart::exchange(other._is_blocking, true);
		return *this;
	}

	/*##### Socket operations #####*/
	bool isValid() const
	{
		return _handle != port_layer::invalid_handle;
	}

	int close()
	{
		int ret = -1;
		if (isValid()) {
			ret = port_layer::close_socket(_handle);
			_handle = port_layer::invalid_handle;
		}
		return ret;
	}

	port_layer::handle_t getNative() const
	{
		return _handle;
	}

	port_layer::handle_t release()
	{
		return mart::exchange(_handle, port_layer::invalid_handle);
	}

	/* ###### send / rec ############### */

	auto send(mart::ConstMemoryView data, int flags) -> port_layer::txrx_size_t
	{
		return ::send(_handle, data.asConstCharPtr(), static_cast<port_layer::txrx_size_t>(data.size()), flags);
	}

	template<class AddrT>
	auto sendto(mart::ConstMemoryView data, int flags, const AddrT& addr) -> port_layer::txrx_size_t
	{
		return ::sendto(_handle, data.asConstCharPtr(), static_cast<port_layer::txrx_size_t>(data.size()), flags, asSockAddrPtr(addr), sizeof(addr));
	}

	auto recv(mart::MemoryView buffer, int flags) -> std::pair<mart::MemoryView, int>
	{
		auto ret = ::recv(_handle, buffer.asCharPtr(), static_cast<port_layer::txrx_size_t>(buffer.size()), flags);
		if (ret >= 0) {
			return{ buffer.subview(0,ret),0 };
		} else {
			return{ mart::MemoryView{}, errno };
		}
	}

	template<class AddrT>
	auto recvfrom(mart::MemoryView buffer, int flags, AddrT& src_addr) -> std::pair<mart::MemoryView, int>
	{
		port_layer::address_len_t len = sizeof(src_addr);
		port_layer::txrx_size_t ret = ::recvfrom(_handle, buffer.asCharPtr(), static_cast<port_layer::txrx_size_t>(buffer.size()), flags, asSockAddrPtr(src_addr), &len);
		if (ret >= 0 && len == sizeof(src_addr)) {
			return{ buffer.subview(0,ret), 0 };
		} else {
			return{ mart::MemoryView{}, errno };
		}
	}

	/* ###### connection related ############### */

	template<class AddrT>
	int bind(const AddrT& addr)
	{
		return ::bind(_handle, asSockAddrPtr(addr), sizeof(AddrT));
	}

	template<class AddrT>
	int connect(const AddrT& addr)
	{
		return ::connect(_handle, asSockAddrPtr(addr), sizeof(addr));
	}
	int listen(int backlog = 10)
	{
		return  ::listen(_handle, backlog);
	}

	template<class AddrT>
	Socket accept(AddrT& remote_addr)
	{
		port_layer::address_len_t len = sizeof(remote_addr);
		Socket h{ ::accept(_handle, asSockAddrPtr(remote_addr), &len) };
		if (len == sizeof(remote_addr)) {
			return h;
		} else {
			return Socket{};
		}
	}

	Socket accept()
	{
		port_layer::address_len_t len = 0;
		port_layer::handle_t h = ::accept(_handle, nullptr, &len);
		return Socket(h);
	}

	/* ###### Configuration ############### */
	//TODO: could actually check mapping between option data type and optname
	template<class T>
	int setsockopt(int level, int optname, const T& option_data)
	{
		auto opmem = mart::asBytes(option_data);
		return ::setsockopt(_handle, level, optname, opmem.asConstCharPtr(), static_cast<port_layer::txrx_size_t>(opmem.size()));
	}
	template<class T>
	int getsockopt(int level, int optname, T& option_data)
	{
		port_layer::address_len_t optlen = sizeof(option_data);
		return ::getsockopt(_handle, level, optname, reinterpret_cast<char*>(&option_data), &optlen);
	}
	bool setBlocking(bool should_block)
	{
		if (port_layer::set_blocking(_handle, should_block)) {
			_is_blocking = should_block;
			return true;
		}
		return false;
	}
	void setTxTimeout(std::chrono::microseconds timeout)
	{
	#ifdef MBA_UTILS_USE_WINSOCKS
		DWORD to_ms = static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
		this->setsockopt(SOL_SOCKET, SO_SNDTIMEO, to_ms);
	#else
		auto to = nw::to_timeval(timeout);
		this->setsockopt(SOL_SOCKET, SO_SNDTIMEO, to);
	#endif
	}

	void setRxTimeout(std::chrono::microseconds timeout)
	{
	#ifdef MBA_UTILS_USE_WINSOCKS
		DWORD to_ms = static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
		this->setsockopt(SOL_SOCKET, SO_RCVTIMEO, to_ms);
	#else
		auto to = nw::to_timeval(timeout);
		this->setsockopt(SOL_SOCKET, SO_RCVTIMEO, to);
	#endif
	}
	std::chrono::microseconds getTxTimeout()
	{
		timeval to;
		this->getsockopt(SOL_SOCKET, SO_SNDTIMEO, to);
		return nw::from_timeval<std::chrono::microseconds>(to);
	}
	std::chrono::microseconds getRxTimeout()
	{
		timeval to;
		this->getsockopt(SOL_SOCKET, SO_RCVTIMEO, to);
		return nw::from_timeval<std::chrono::microseconds>(to);
	}
	bool isBlocking() const
	{
		//XXX: acutally query the native socket
		return _is_blocking;
	}


private:
	bool _open(Domain domain, TransportType type, int protocol)
	{
		if (_impl_details_nw::startUp() == false) {
			return false;
		}
		_handle = ::socket(mart::toUType(domain), mart::toUType(type), protocol);
		return _handle != port_layer::invalid_handle;
	}
	port_layer::handle_t _handle = port_layer::invalid_handle;
	bool _is_blocking = true;
};

}//socks
}//nw
}//experimental
}//mart


#endif
