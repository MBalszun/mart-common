#include <mart-common/StringViewOstream.h>
#include <mart-netlib/ip.hpp>

#include <catch2/catch.hpp>

TEST_CASE( "ipv4_address_roundtrips_throug string", "[net]" )
{
	std::string_view         ref = "101.112.123.134";
	mart::nw::ip::address_v4 addr( ref );
	CHECK( addr.asString() == ref );
}