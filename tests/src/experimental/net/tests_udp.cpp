#include <mart-common/experimental/net/udp.hpp>

#include <catch2/catch.hpp>

#include <iostream>
TEST_CASE( "udp_socket_members_do_compile", "[net]" )
{
	using namespace mart::nw::ip;
	udp::endpoint e1 = {mart::nw::ip::address_any, mart::nw::ip::port_nr {5555}};
	udp::endpoint e2 = {mart::nw::ip::address_local_host, mart::nw::ip::port_nr {5455}};
	udp::endpoint e3 {"127.0.0.1:3435"};
	udp::Socket   s1;

	udp::Socket s2 {};

	udp::Socket s3( e1, e3 );
	udp::Socket s4( std::move( s1 ) );
	s2 = std::move( s4 );

	[[maybe_unused]] mart::nw::socks::Socket& raw_socket = s2.getRawSocket();

	udp::endpoint e4 = {mart::nw::ip::address_any, mart::nw::ip::port_nr {2553}};
	s2.bind( e4 );
	//socket can't be rebinded to a different one
	CHECK( !s2.try_bind( e4 ) );
	s2.connect( e3 );
 	CHECK( s2.try_connect( e3 ) );

	CHECK( s2.getLocalEndpoint() == e4 );
	CHECK( s2.getRemoteEndpoint() == e3 );

	s2.send( mart::view_bytes( 5 ) );
	s2.sendto( mart::view_bytes( 5 ), e2 );

	using namespace std::chrono_literals;
	s2.setRxTimeout( 1ms );
	s2.setTxTimeout( 2ms );
	// TODO Check, why those fail on travis
	//CHECK( s2.getRxTimeout() == 1ms );
	//CHECK( s2.getTxTimeout() == 2ms );

	int buffer = 0;
	udp::endpoint e5;
	CHECK( !s2.rec( mart::view_bytes_mutable( buffer ) ).isValid() );
	CHECK( !s2.recvfrom( mart::view_bytes_mutable( buffer ), e5 ).isValid() );

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
