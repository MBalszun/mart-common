#include <mart-netlib/udp.hpp>

#include <mart-common/PrintWrappers.h>

#include <catch2/catch.hpp>

TEST_CASE( "udp_socket_simple_member_check1", "[net]" )
{
	using namespace mart::nw::ip;
	udp::endpoint e1 = {mart::nw::ip::address_any, mart::nw::ip::port_nr{1573}};
	udp::endpoint e2 = {mart::nw::ip::address_local_host, mart::nw::ip::port_nr{1455}};
	udp::endpoint e3{"127.0.0.1:3435"};
	udp::Socket   s1;

	udp::Socket s2{};

	udp::Socket s3( e1, e3 );
	s3.close();
	udp::Socket s4( std::move( s1 ) );
	s2 = std::move( s4 );

	[[maybe_unused]] mart::nw::socks::RaiiSocket& raw_socket = s2.getRawSocket();

	s2.bind( e1 );
	// socket can't be rebinded to a different one
	CHECK( !s2.try_bind( e1 ) );
	CHECK_NOTHROW( s2.connect( e2 ) );
	CHECK( s2.get_local_endpoint() == e1 );
	CHECK( s2.get_remote_endpoint() == e2 );

	CHECK( s2.try_connect( e3 ) );

	CHECK( s2.get_local_endpoint() == e1 );
	CHECK( s2.get_remote_endpoint() == e3 );

	CHECK_NOTHROW( s2.send( mart::view_bytes( 5 ) ) );
	// TODO calling sendto on a connected socket has different behavior on linux and windows
	// CHECK_THROWS( s2.sendto( mart::view_bytes( 5 ), e2 ) );

	CHECK_THROWS( s2.sendto( mart::view_bytes( 5 ), e1 ) ); // invalid target address
}

TEST_CASE( "udp_socket_simple_member_check2", "[net]" )
{
	using namespace mart::nw::ip;
	udp::Socket s{};

	CHECK( s.is_blocking() );

	CHECK_NOTHROW( s.bind( udp::endpoint{"127.0.0.1:3446"} ) );
	using namespace std::chrono_literals;
	CHECK( s.set_rx_timeout( 32ms ) ); // TODO: on some platforms (travis), must be multiple of 4ms
	CHECK( s.set_tx_timeout( 20ms ) );
	CHECK( s.get_tx_timeout() == 20ms );
	CHECK( s.get_rx_timeout() == 32ms );

	int buffer = 0;
	CHECK( !s.try_recv( mart::view_bytes_mutable( buffer ) ).isValid() );

	CHECK( s.try_send( mart::view_bytes( buffer ) ).size() != 0 );
	CHECK_THROWS( s.send( mart::view_bytes( buffer ) ).isValid() );
	{
		udp::endpoint e4{};
		CHECK( s.try_sendto( mart::view_bytes( buffer ), e4 ).size() != 0 );
		CHECK_THROWS( s.sendto( mart::view_bytes( buffer ), e4 ).isValid() );
	}

	CHECK( !s.try_recv( mart::view_bytes_mutable( buffer ) ).isValid() );
	CHECK( !s.try_recvfrom( mart::view_bytes_mutable( buffer ) ).data.isValid() );

	CHECK( !s.recv( mart::view_bytes_mutable( buffer ) ).isValid() );
	CHECK( !s.recvfrom( mart::view_bytes_mutable( buffer ) ).data.isValid() );

	s.clearRxBuff();

	CHECK( s.set_blocking( false ) );
	CHECK( !s.is_blocking() );

	s.close();
	CHECK( !s.is_valid() );
}

TEST_CASE( "invalid_endpoint_strings_fail" )
{
	using namespace mart::nw::ip;
	CHECK_THROWS( udp::endpoint{"127.0.0.1"} );
	CHECK_THROWS( udp::endpoint{"127.0.0:5"} );
	CHECK_THROWS( udp::endpoint{"127005"} );
	CHECK_THROWS( udp::endpoint{"127.0.0.1:66999"} );
	CHECK_THROWS( udp::endpoint{"1.333.0.1:669"} );
}
