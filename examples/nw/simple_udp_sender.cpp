
//#define USE_NATIVE_SOCKET_API

#ifdef USE_NATIVE_SOCKET_API
	#include <stdexcept>

	#include <netinet/ip.h>
	#include <sys/socket.h>
	#include <unistd.h>
#else
	#include <mart-common/experimental/network/udp.h>
#endif

constexpr char message[] = {"Hello World!"};

int main()
{
#ifdef USE_NATIVE_SOCKET_API
	sockaddr_in address {};
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = INADDR_LOOPBACK;
	address.sin_port        = htons( 6565 );

	auto handle = ::socket( AF_INET, SOCK_DGRAM, 0 );
	if( handle == -1 ) {
		close( handle );
		throw std::exception {};
	}

	sendto( handle, &message, sizeof( message ), 0, (sockaddr*)&address, sizeof( address ) );
	close( handle );
#else
	using namespace mart::experimental;
	auto dest = nw::ip::udp::parse_v4_endpoint( "127.0.0.1:6345" ).value();

	nw::ip::udp::Socket{}.sendto( mart::view_bytes( message ), dest );
#endif
}