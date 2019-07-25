#include <mart-netlib/udp.hpp>

#include <mart-common/PrintWrappers.h>

#include <catch2/catch.hpp>

#include <iostream>
TEST_CASE( "udp_socket_simple_member_check1", "[net]" )
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
	// socket can't be rebinded to a different one
	CHECK( !s2.try_bind( e1 ) );
	CHECK_NOTHROW( s2.connect( e2 ) );
	CHECK( s2.getLocalEndpoint() == e1 );
	CHECK( s2.getRemoteEndpoint() == e2 );


	CHECK( s2.try_connect( e3 ) );

	CHECK( s2.getLocalEndpoint() == e1 );
	CHECK( s2.getRemoteEndpoint() == e3 );

	CHECK_NOTHROW( s2.send( mart::view_bytes( 5 ) ) );
	CHECK_NOTHROW( s2.sendto( mart::view_bytes( 5 ), e2 ) );

	CHECK_THROWS( s2.sendto( mart::view_bytes( 5 ), e1 ) ); // invalid target address
}

namespace {
template<class Rep, class Period>
std::ostream& operator<<( std::ostream& out, std::chrono::duration<Rep, Period> dur )
{
	out << mart::sformat( dur );
	return out;
}
} // namespace
TEST_CASE( "udp_socket_simple_member_check2", "[net]" )
{
	using namespace mart::nw::ip;
	udp::Socket s {};

	CHECK( s.isBlocking() );

	CHECK_NOTHROW( s.bind( udp::endpoint {"127.0.0.1:3446"} ) );

	using namespace std::chrono_literals;
	CHECK( s.setRxTimeout( 1ms ) );
	CHECK( s.setTxTimeout( 2ms ) );
	CHECK( s.getTxTimeout() == 2ms );
	CHECK( s.getRxTimeout() == 1ms );

	int buffer = 0;
	CHECK( !s.rec( mart::view_bytes_mutable( buffer ) ).isValid() );

	udp::endpoint e4;
	CHECK( !s.recvfrom( mart::view_bytes_mutable( buffer ), e4 ).isValid() );

	s.clearRxBuff();

	CHECK( s.setBlocking( false ) );
	CHECK( !s.isBlocking() );

	s.close();
	CHECK( !s.isValid() );
}

TEST_CASE( "invalid_endpoint_strings_fail" )
{
	using namespace mart::nw::ip;
	CHECK_THROWS( udp::endpoint {"127.0.0.1"} );
	CHECK_THROWS( udp::endpoint {"127.0.0:5"} );
	CHECK_THROWS( udp::endpoint {"127005"} );
	CHECK_THROWS( udp::endpoint {"127.0.0.1:66999"} );
	CHECK_THROWS( udp::endpoint {"1.333.0.1:669"} );
}
