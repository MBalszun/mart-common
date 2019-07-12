#include <mart-common/experimental/net/Socket.hpp>

#include <catch2/catch.hpp>

TEST_CASE( "net_socket_is_blocking_after_construction", "[net]" )
{
	using namespace mart::nw;
	//socks::port_layer::waInit();
	socks::Socket socket( socks::Domain::Inet6, socks::TransportType::datagram );
	CHECK( socket.isValid() );
	CHECK( socket.isBlocking() );
	CHECK( socket.setBlocking( true ) );
	CHECK( socket.isBlocking() );
	CHECK( socket.setBlocking( false ) );
	CHECK( !socket.isBlocking() );
	CHECK( socket.setBlocking( true ) );
	CHECK( socket.isBlocking() );
	CHECK( socket.setBlocking( false ) );
	CHECK( !socket.isBlocking() );

	socket = socks::Socket( socks::Domain::Inet, socks::TransportType::datagram );
	CHECK( socket.isBlocking() );
	CHECK( socket.setBlocking( true ) );
	CHECK( socket.isBlocking() );
	CHECK( socket.setBlocking( false ) );
	CHECK( !socket.isBlocking() );
	CHECK( socket.setBlocking( true ) );
	CHECK( socket.isBlocking() );
	CHECK( socket.setBlocking( false ) );
	CHECK( !socket.isBlocking() );
}