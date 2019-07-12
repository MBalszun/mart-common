#include <mart-common/experimental/net/udp.hpp>

#include <catch2/catch.hpp>

#include <iostream>
TEST_CASE( "udp_socket_simple_member_check", "[net]" )
{
	using namespace mart::nw::ip;
	udp::endpoint e1 = {mart::nw::ip::address_any, mart::nw::ip::port_nr {1573}};
	udp::endpoint e2 = {mart::nw::ip::address_local_host, mart::nw::ip::port_nr {1455}};
	udp::endpoint e3 {"127.0.0.1:3435"};
	udp::Socket   s1;

	udp::Socket s2 {};

	udp::Socket s3( e1, e3 );
	s3.close();
	udp::Socket s4( std::move( s1 ) );
	s2 = std::move( s4 );

	[[maybe_unused]] mart::nw::socks::Socket& raw_socket = s2.getRawSocket();

	s2.bind( e1 );
	//socket can't be rebinded to a different one
	CHECK( !s2.try_bind( e1 ) );
	s2.connect( e2 );
 	CHECK( s2.try_connect( e2 ) );

	CHECK( s2.getLocalEndpoint() == e1 );
	CHECK( s2.getRemoteEndpoint() == e2 );

	s2.send( mart::view_bytes( 5 ) );
	s2.sendto( mart::view_bytes( 5 ), e1 );

	using namespace std::chrono_literals;
	CHECK( s2.setRxTimeout( 1ms ) );
	CHECK( s2.setTxTimeout( 2ms ) );
	CHECK( s2.getRxTimeout() == 1ms );
	CHECK( s2.getTxTimeout() == 2ms );

	int buffer = 0;
	udp::endpoint e4;
	CHECK( !s2.rec( mart::view_bytes_mutable( buffer ) ).isValid() );
	CHECK( !s2.recvfrom( mart::view_bytes_mutable( buffer ), e4 ).isValid() );

	s2.clearRxBuff();

	CHECK( s2.isBlocking() );
	CHECK( s2.setBlocking( false ) );
	CHECK( !s2.isBlocking() );

	s2.close();
	CHECK( !s2.isValid() );

}

TEST_CASE("invalid_endpoint_strings_fail") {
	using namespace mart::nw::ip;
	CHECK_THROWS(udp::endpoint {"127.0.0.1"});
	CHECK_THROWS(udp::endpoint {"127.0.0:5"});
	CHECK_THROWS(udp::endpoint {"127005"});
	CHECK_THROWS(udp::endpoint {"127.0.0.1:66999"});
	CHECK_THROWS(udp::endpoint {"1.333.0.1:669"});

}
