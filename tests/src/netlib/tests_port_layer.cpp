#include <mart-netlib/port_layer.hpp>

#include <catch2/catch.hpp>

#include <iostream>

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

	pl::handle_t s2
		= pl::socket( socks::Domain::Inet, socks::TransportType::Datagram ).value_or( pl::handle_t::Invalid );
	CHECK( s2 != pl::handle_t::Invalid );

	pl::SockaddrIn addr{};

	CHECK( set_blocking( s2, false ) );
	CHECK( !accept( s2 ) );
	CHECK( !accept( s2, addr ) );

	// CHECK( connect( s2,addr ) ); // TODO: connect to empty works on some platforms but not on all
	connect( s2, addr );
	// CHECK( !bind( s2, addr ) );
	bind( s2, addr );
	// CHECK( !listen( s2, 10 ) );
	listen( s2, 10 );
}

TEST_CASE( "net_port-layer_getaddrinfo" )
{
	socks::AddrInfoHints hints{};
	hints.flags    = 0;
	hints.family   = socks::Domain::Unspec;
	hints.socktype = socks::TransportType::Stream;

	auto info1 = pl::getaddrinfo( "www.google.de", "https", hints );

	CHECK( info1.success() );
	auto& sockaddr_list = info1.value();

	for( const auto& e : sockaddr_list ) {

		socks::ReturnValue<pl::handle_t> res = pl::socket( e.family, e.socktype, e.protocol );
		CHECK( res.success() );
		auto handle = res.value();

		std::cout << "Connecting to (to_string)       : " << pl::to_string( e.addr->to_Sockaddr() ) << std::endl;

		char buffer[30]{};
		pl::inet_net_to_pres( e.addr->to_Sockaddr().to_native_ptr(), buffer, sizeof( buffer ) );
		std::cout << "Connecting to (inet_net_to_pres): " << &( buffer[0] ) << std::endl;

		auto con_res = pl::connect( handle, e.addr->to_Sockaddr() );
		if( !con_res.success() ) { std::cout << "Failed with error code: " << con_res.raw_value() << std::endl; }

		CHECK( pl::close_socket( handle ).success() );
	}
}
