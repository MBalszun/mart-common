#include <mart-common/experimental/network/Socket.h>

#include <catch2/catch.hpp>

TEST_CASE( "experimental_exp_socket_is_blocking_after_construction", "[experimental][ranges]" )
{
	using namespace mart::experimental::nw;
	//socks::port_layer::waInit();
	socks::Socket socket( socks::Domain::inet6, socks::TransportType::datagram );
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

	socket = socks::Socket( socks::Domain::inet, socks::TransportType::datagram );
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