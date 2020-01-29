
//#define USE_NATIVE_SOCKET_API
//#define USE_OLD_NETLIB

#ifdef USE_NATIVE_SOCKET_API

#include <stdexcept>
#ifdef _MSC_VER

#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib" )

inline void close_socket( SOCKET h )
{
	closesocket( h );
}

#else

#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

inline void close_socket( int h )
{
	close( h );
}

#endif

#else // USE_NATIVE_SOCKET_API

#ifdef USE_OLD_NETLIB
#include <mart-common/experimental/network/udp.h>
#else
#include <mart-netlib/udp.hpp>
#endif

#endif // USE_NATIVE_SOCKET_API

constexpr char message[] = {"Hello World!"};

int main()
{
#ifdef USE_NATIVE_SOCKET_API

	sockaddr_in address{};
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = INADDR_LOOPBACK;
	address.sin_port        = htons( 6565 );

	auto handle = ::socket( AF_INET, SOCK_DGRAM, 0 );
	if( handle == -1 ) {
		close_socket( handle );
		throw std::exception{};
	}

	sendto( handle, message, sizeof( message ), 0, (sockaddr*)&address, sizeof( address ) );

	close_socket( handle );

#else

#ifdef USE_OLD_NETLIB
	namespace udp = mart::experimental::nw::ip::udp;
	auto dest     = udp::parse_v4_endpoint( "127.0.0.1:6345" ).value();
	udp::Socket{}.sendto( mart::view_bytes( message ), dest );
#else
	namespace udp = mart::nw::ip::udp;
	auto dest     = udp::try_parse_v4_endpoint( "127.0.0.1:6345" ).value();
	udp::Socket{}.try_sendto( mart::view_bytes( message ), dest );
#endif
#endif
}