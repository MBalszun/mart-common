#include <mart-common/StringView.h>

#include <catch2/catch.hpp>

#include <string>



TEST_CASE( "various", "[StringView]" )
{
	CHECK( "Hello" == mart::StringView( "Hello" ) );
	CHECK( "Hello" == mart::StringView( "Hello" ).to_string() );
	CHECK( "Hello" == mart::StringView( "1 Hello 123" ).substr( 2, 5 ) );
	CHECK( "Hello 123" == mart::StringView( "1 Hello 123" ).substr( 2 ) );

}

