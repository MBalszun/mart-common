#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_PORT_LAYER_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_PORT_LAYER_H
/*
* port_layer.h
*
*  Created on: 2016-09-29
*      Author: Michael Balszun <michael.balszun@tum.de>
*
*      This header provides a compatibility layer by defining basic functions and
*      types that differ between windows and linux
*/

//detect windows os. TODO: use other guards if necessary
#ifdef _MSC_VER
#define MBA_UTILS_USE_WINSOCKS

//assume little endian for windows
#define MBA_ORDER_LITTLE_ENDIAN 1
#define MBA_BYTE_ORDER			MBA_ORDER_LITTLE_ENDIAN
#else
//use gcc primitives
#define MBA_ORDER_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define MBA_BYTE_ORDER			__BYTE_ORDER__
#endif

//Include OS-specific headers
#ifdef MBA_UTILS_USE_WINSOCKS

struct IUnknown; //This declaration will be needed when translating windows headers with clang/c2
//Including Windows.h (indirectly) tentds to import some nasty macros. in particular min and max
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#define MBA_UTILS_DEFINED_WIN_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
	#define NOMINMAX
	#define MBA_UTILS_DEFINED_NOMINMAX
#endif

#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#undef ERROR

#ifdef MBA_UTILS_DEFINED_WIN_LEAN_AND_MEAN
	#undef WIN32_LEAN_AND_MEAN
#endif

#ifdef MBA_UTILS_DEFINED_NOMINMAX
	#undef NOMINMAX
#endif

#else
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <sys/un.h>
	#include <arpa/inet.h>
	#include <unistd.h> //close
	#include <fcntl.h>
#endif

namespace mart {
namespace experimental {
namespace nw {
namespace socks {
namespace port_layer {

//Define aliases for platform specific types and values
#ifdef MBA_UTILS_USE_WINSOCKS
using handle_t = SOCKET;
using address_len_t = int;
using txrx_size_t = int;
static constexpr handle_t invalid_handle = INVALID_SOCKET;
#else
using handle_t = int;
using address_len_t = socklen_t;
using txrx_size_t = ssize_t;
static constexpr handle_t invalid_handle = -1;
#endif // MBA_UTILS_USE_WINSOCKS

//Wrapper functions for socket related functions, that are specific to a certain platform
inline bool set_blocking(handle_t socket, bool should_block)
{
	bool ret = true;
#ifdef MBA_UTILS_USE_WINSOCKS
	// from http://stackoverflow.com/questions/5489562/in-win32-is-there-a-way-to-test-if-a-socket-is-non-blocking/33087879
	/// @note windows sockets are created in blocking mode by default
	// currently on windows, there is no easy way to obtain the socket's current blocking mode since WSAIsBlocking was deprecated
	u_long non_blocking = should_block ? 0 : 1;
	ret = NO_ERROR == ioctlsocket(socket, FIONBIO, &non_blocking);
#else
	const int flags = fcntl(socket, F_GETFL, 0);
	if ((flags & O_NONBLOCK) == !should_block) { return ret; }
	ret = 0 == fcntl(socket, F_SETFL, should_block ? flags & ~O_NONBLOCK : flags | O_NONBLOCK);
#endif
	return ret;
}

inline int close_socket(handle_t handle)
{
#ifdef MBA_UTILS_USE_WINSOCKS
	return ::closesocket(handle);
#else
	return ::close(handle); //in linux, a socket is just another file descriptor
#endif
}

inline int getLastSocketError()
{
#ifdef MBA_UTILS_USE_WINSOCKS
	return WSAGetLastError();
#else
	return errno;
#endif
}


inline bool waInit()
{
#ifdef MBA_UTILS_USE_WINSOCKS
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return false;
	}
	return true;
#else
	return true; //on linux we don't have to initialize anything
#endif
}

} //port_layer
} //sock

namespace ip {
namespace port_layer {

	inline const char *inet_net_to_pres(int af, const void *src, char *dst, size_t size)
	{
		#ifdef MBA_UTILS_USE_WINSOCKS //detect windows os - use other guards if necessary
			return InetNtop(af, const_cast<void*>(src), dst, size);
		#else
			return inet_ntop(af, src, dst, size);
		#endif
	}

	inline int inet_pres_to_net(int af, const char *src, void *dst)
	{
		#ifdef MBA_UTILS_USE_WINSOCKS //detect windows os - use other guards if necessary
			return InetPton(af, src, dst);
		#else
			return inet_pton(af, src, dst);
		#endif
	}
} //port_layer
}//ip
} //nw
} //experimental
} //mba

//we have to define sockaddr_un type on windows
//XXX find way to create compiletime error, if used on windows
#ifdef MBA_UTILS_USE_WINSOCKS
struct sockaddr_un {
private:
	//prevent instantiation on windows instantiate
	sockaddr_un(){}
	~sockaddr_un(){}
};

#endif



#endif //header guard
