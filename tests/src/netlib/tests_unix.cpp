#include <mart-netlib/unix.hpp>

#include <mart-common/PrintWrappers.h>

#include <catch2/catch.hpp>

#include <filesystem>
#include <iostream>

TEST_CASE( "unix_domain_socket_simple_member_check1", "[net]" )
{

	mart::nw::un::Socket sock1;
	std::cout << "Empty socket created" << std::endl;
	mart::nw::un::Socket sock2(                    //
		mart::ConstString( "__tmp__/sock2.local" ), //
		mart::ConstString( "__tmp__/sock2.remote" ) //
	);
}