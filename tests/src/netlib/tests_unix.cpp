#include <mart-netlib/unix.hpp>

#include <mart-common/PrintWrappers.h>

#include <catch2/catch.hpp>

#include <filesystem>
#include <iostream>

TEST_CASE( "unix_domain_socket_simple_member_check1", "[.][net][unix_domain_socket]" )
{

	mart::nw::un::Socket sock1;
	sock1.bind( mart::ConstString("sock1") );
	mart::nw::un::Socket sock2(                    //
		mart::ConstString( "sock2" ), //
		mart::ConstString( "sock1" ) //
	);
}