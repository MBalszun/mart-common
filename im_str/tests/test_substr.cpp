#include <im_str/im_str.hpp>

#include "include_catch.hpp"

using namespace ::mba;

TEST_CASE( "Substring", "[im_str]" )
{
	im_str cs = "HelloWorld";
	{
		auto s = cs.substr( 5 );
		REQUIRE( s == "World" );
	}
	{
		auto s = cs.substr( 5, 2 );
		REQUIRE( s == "Wo" );
	}
	{
		std::string_view sv( cs );
		auto             ssv = cs.substr( 5 );
		auto             s   = cs.substr( ssv );
		auto             s2  = cs.substr( sv );

		REQUIRE( s == ssv );
		REQUIRE( s2 == cs );
	}
	{
		auto s = cs.substr( cs.begin() + 2, cs.end() );
		REQUIRE( s == "lloWorld" );
	}
}
