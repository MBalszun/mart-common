#include <mart-netlib/tcp.hpp>

#include <mart-common/PrintWrappers.h>

#include <catch2/catch.hpp>

#include <future>
#include <iostream>


namespace mart::nw::ip::tcp {

std::ostream& operator<<( std::ostream& out, mart::nw::ip::tcp::endpoint ep )
{
	out << ep.toStringEx();
	return out;
}

} // namespace


TEST_CASE( "tcp_socket_simple_exchange", "[net]" )
{
	using namespace mart::nw::ip;
	tcp::endpoint e1 = { mart::nw::ip::address_any, mart::nw::ip::port_nr{ 1583 } };
	tcp::endpoint e2 = { mart::nw::ip::address_local_host, mart::nw::ip::port_nr{ 1485 } };
	tcp::endpoint e3{ "127.0.0.1:3495" };
	tcp::Socket   s1;

	tcp::Socket s2{};


	tcp::Socket s4( std::move( s1 ) );
	s2 = std::move( s4 );

	[[maybe_unused]] mart::nw::socks::RaiiSocket& raw_socket = s2.as_raii_socket();

	CHECK_NOTHROW( s2.bind( e1 ) );
	// socket can't be rebinded to a different one
	CHECK_THROWS( s2.bind( e2 ) );
	CHECK_THROWS( s2.connect( e2 ) );
	CHECK( s2.get_local_endpoint() == e1 );
	CHECK( !s2.get_remote_endpoint().valid() );

	CHECK_THROWS( s2.send( mart::view_bytes( 5 ) ) );
}

TEST_CASE( "tcp_simple_exchange", "[net]" )
{
	using namespace mart::nw::ip;
	tcp::endpoint e1 = { mart::nw::ip::address_local_host, mart::nw::ip::port_nr{ 1584 } };
	tcp::endpoint e2 = { mart::nw::ip::address_local_host, mart::nw::ip::port_nr{ 1486 } };

	auto s1_future = std::async( [&] {
		tcp::Acceptor ac( e1 );
		return ac.accept( std::chrono::milliseconds( 1000 ) );
	} );

	tcp::Socket s2;
	s2.bind( e2 );
	s2.connect( e1 );

	auto s1 = s1_future.get();
	CHECK( s1.is_valid() );

	CHECK( s2.get_remote_endpoint() == e1 );
	CHECK( s1.get_remote_endpoint() == e2 );

	const int data_orig = 0xffa1;
	int       data_rec{};
	s1.send( mart::view_bytes( data_orig ) );
	auto rec = s2.recv( mart::view_bytes_mutable( data_rec ) );

	CHECK( rec.size_inBytes() == sizeof( int ) );
	CHECK( data_orig == data_rec );

}