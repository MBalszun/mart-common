/*
* NativesocketWrapper.h
*
*  Created on: 2016-09-29
*      Author: Michael Balszun <michael.balszun@tum.de>
*
*      This file provides the NativeSocketWrapper class
*/

#ifndef LIBS_MART_COMMON_EXPERIMENTAL_NW_NATIVE_SOCKET_WRAPPER_H_
#define LIBS_MART_COMMON_EXPERIMENTAL_NW_NATIVE_SOCKET_WRAPPER_H_
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */

/* Proprietary Library Includes */
#include "../../cpp_std/utility.h"
#include "../../ArrayView.h"

/* Project Includes */
#include "basic_types.h"
#include "port_layer.h"


namespace mart {
namespace experimental {
namespace nw {

namespace _impl_details_nw {

//on windows, the wa system has to be initialized before sockets can be used
inline bool startUp() {
	const static bool isInit = port::sock::waInit();
	return isInit;
}

} //_impl_details_nw

enum class SocketDomain {
	local = AF_UNIX,
	inet = AF_INET,
	inet6 = AF_INET6,
};

enum class SocketType {
	stream = SOCK_STREAM,
	datagram = SOCK_DGRAM,
	seqpacket = SOCK_SEQPACKET,
};



template<SocketDomain, SocketType>
struct socket_traits {};

template<SocketType Type>
struct socket_traits<SocketDomain::local, Type> {
	using address_t = sockaddr_un;
};
template<SocketType Type>
struct socket_traits<SocketDomain::inet, Type> {
	using address_t = sockaddr_in;
};

template<SocketType Type>
struct socket_traits<SocketDomain::inet6, Type> {
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
class NativeSocketWrapper {
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
	constexpr NativeSocketWrapper() = default;
	explicit NativeSocketWrapper(port::sock::handle_type handle) :
		_handle(handle)
	{}
	NativeSocketWrapper(SocketDomain domain, SocketType type, int protocol = 0)
	{
		_open(domain, type, protocol);
	}
	~NativeSocketWrapper()
	{
		close();
	}

	/*##### Special member functions #####*/
	NativeSocketWrapper(const NativeSocketWrapper& other) = delete;
	NativeSocketWrapper& operator=(const NativeSocketWrapper& other) = delete;

	NativeSocketWrapper(NativeSocketWrapper&& other) :
		_handle		{ mart::exchange(other._handle, port::sock::invalid_handle) },
		_is_blocking{ mart::exchange(other._is_blocking, true) }
	{}

	NativeSocketWrapper& operator=(NativeSocketWrapper&& other)
	{
		close();
		_handle			= mart::exchange(other._handle, port::sock::invalid_handle);
		_is_blocking	= mart::exchange(other._is_blocking, true);
		return *this;
	}

	/*##### Socket operations #####*/
	bool isValid() const
	{
		return _handle != port::sock::invalid_handle;
	}

	int close()
	{
		int ret = -1;
		if (isValid()) {
			ret = port::sock::close_socket(_handle);
			_handle = port::sock::invalid_handle;
		}
		return ret;
	}

	port::sock::handle_type getNative() const
	{
		return _handle;
	}

	port::sock::handle_type release()
	{
		return mart::exchange(_handle, port::sock::invalid_handle);
	}

	/* ###### send / rec ############### */

	auto send(mart::ConstMemoryView data, int flags) -> port::sock::txrx_size_type
	{
		return ::send(_handle, data.asConstCharPtr(), data.size_inBytes(), flags);
	}

	template<class AddrT>
	auto sendto(mart::ConstMemoryView data, int flags, const AddrT& addr) -> nw::port::sock::txrx_size_type
	{
		return ::sendto(_handle, data.asConstCharPtr(), data.size_inBytes(), flags, asSockAddrPtr(addr), sizeof(addr));
	}

	auto recv(mart::MemoryView buffer, int flags) -> std::pair<nw::port::sock::txrx_size_type, mart::MemoryView>
	{
		auto ret = ::recv(_handle, buffer.asCharPtr(), buffer.size(), flags);
		if (ret >= 0) {
			return{ ret,buffer.subview(0,ret) };
		} else {
			return{ ret, mart::MemoryView{} };
		}
	}

	template<class AddrT>
	auto recvfrom(mart::MemoryView buffer, int flags, AddrT& src_addr) -> std::pair<nw::port::sock::txrx_size_type, mart::MemoryView>
	{
		nw::port::sock::address_len_type len = sizeof(src_addr);
		nw::port::sock::txrx_size_type ret = ::recvfrom(_handle, buffer.asCharPtr(), buffer.size(), flags, asSockAddrPtr(src_addr), &len);
		if (ret >= 0 && len == sizeof(src_addr)) {
			return{ ret,buffer.subview(0,ret) };
		} else {
			return{ ret, mart::MemoryView{} };
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
	NativeSocketWrapper accept(AddrT& remote_addr)
	{
		nw::port::sock::address_len_type len = sizeof(remote_addr);
		NativeSocketWrapper h{ ::accept(_handle, asSockAddrPtr(remote_addr), &len) };
		if (len == sizeof(remote_addr)) {
			return h;
		} else {
			return NativeSocketWrapper{};
		}
	}

	NativeSocketWrapper accept()
	{
		nw::port::sock::address_len_type len = 0;
		nw::port::sock::handle_type h = ::accept(_handle, nullptr, &len);
		return NativeSocketWrapper(h);
	}

	/* ###### Configuration ############### */
	//TODO: could actually check mapping between option data type and optname
	template<class T>
	int setsockopt(int level, int optname, const T& option_data)
	{
		auto opmem = mart::viewMemoryConst(option_data);
		return ::setsockopt(_handle, level, optname, opmem.asConstCharPtr(), opmem.size());
	}
	template<class T>
	int getsockopt(int level, int optname, T& option_data)
	{
		nw::port::sock::address_len_type optlen = sizeof(option_data);
		return ::getsockopt(_handle, level, optname, reinterpret_cast<char*>(&option_data), &optlen);
	}
	bool setBlocking(bool should_block)
	{
		if (nw::port::sock::set_blocking(_handle, should_block)) {
			_is_blocking = should_block;
			return true;
		}
		return false;
	}
	bool isBlocking() const
	{
		//XXX: acutally query the native socket
		return _is_blocking;
	}


private:
	bool _open(SocketDomain domain, SocketType type, int protocol)
	{
		if (_impl_details_nw::startUp() == false) {
			return false;
		}
		_handle = ::socket(mart::toUType(domain), mart::toUType(type), protocol);
		return _handle != port::sock::invalid_handle;
	}
	port::sock::handle_type _handle = port::sock::invalid_handle;
	bool _is_blocking = true;
};


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


}//nw
}//experimental
}//mart


#endif
