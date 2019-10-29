#include <mart-netlib/Socket.hpp>

#include <catch2/catch.hpp>

namespace {
	std::ostream& operator<<(std::ostream& out, mart::nw::socks::ErrorCode ec) {
		out << ec.raw_value();
		return out;
	}
}


TEST_CASE( "net_socket_is_blocking_after_construction", "[net]" )
{
	using namespace mart::nw;
	//socks::port_layer::waInit();
	socks::RaiiSocket socket( socks::Domain::Inet6, socks::TransportType::Datagram );
	CHECK( socket.is_valid() );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( true ) );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ) );
	CHECK( !socket.is_blocking() );
	CHECK( socket.set_blocking( true ) );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ) );
	CHECK( !socket.is_blocking() );

	socket = socks::RaiiSocket( socks::Domain::Inet, socks::TransportType::Datagram );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( true ) );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ) );
	CHECK( !socket.is_blocking() );
	CHECK( socket.set_blocking( true ) );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ) );
	CHECK( !socket.is_blocking() );
}