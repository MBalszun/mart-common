#include <mart-netlib/unix.hpp>

#include <mart-common/PrintWrappers.h>

#include <catch2/catch.hpp>

#include <filesystem>
#include <iostream>
#include <stdexcept>

TEST_CASE( "unix_domain_socket_simple_member_check1", "[.][net][unix_domain_socket]" )
{

	mart::nw::un::Socket  sock1;
	std::filesystem::path sock_path1( "sock1" );
	std::filesystem::path sock_path2( "sock2" );

	try {
		std::filesystem::remove( sock_path1 );
	} catch( const std::exception& e ) {
		std::cout << e.what() << std::endl;
	}
	try {
		std::filesystem::remove( sock_path2 );
	} catch( const std::exception& e ) {
		std::cout << e.what() << std::endl;
	}

	sock1.bind( mba::im_zstr( sock_path1.string() ) );
	mart::nw::un::Socket sock2(              //
		mba::im_zstr( sock_path2.string() ), //
		mba::im_zstr( sock_path1.string() )  //
	);
}