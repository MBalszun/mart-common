#include <mart-netlib/port_layer.hpp>

#include <catch2/catch.hpp>

namespace pl    = mart::nw::socks::port_layer;
namespace socks = mart::nw::socks;

// call ech function at least once to ensure the implementation is there
TEST_CASE( "net_port-layer_check_function_implementation_exists" )
{
	[[maybe_unused]] auto nh = pl::to_native( pl::handle_t::Invalid );
	[[maybe_unused]] int  d  = pl::to_native( socks::Domain::Inet );
	[[maybe_unused]] int  t  = pl::to_native( socks::TransportType::Datagram );
	[[maybe_unused]] int  p  = pl::to_native( socks::Protocol::Tcp );

	socks::ReturnValue<pl::handle_t> ts
		= pl::socket( socks::Domain::Inet, socks::TransportType::Datagram, socks::Protocol::Default );
	CHECK( ts.success() );
	CHECK( ts.value_or( pl::handle_t::Invalid ) != pl::handle_t::Invalid );
	pl::close_socket( ts.value_or( pl::handle_t::Invalid ) );

	pl::handle_t s2 = pl::socket( socks::Domain::Inet, socks::TransportType::Datagram )
						  .value_or( pl::handle_t::Invalid );
	CHECK( s2 != pl::handle_t::Invalid );

	pl::SockaddrIn addr {};

	CHECK( set_blocking( s2, false ) );
	CHECK( !accept( s2 ) );
	CHECK( !accept( s2,addr ) );

	CHECK( !connect( s2,addr ) );
	CHECK( !bind( s2, addr ) );
	CHECK( !listen( s2, 10 ) );

}

