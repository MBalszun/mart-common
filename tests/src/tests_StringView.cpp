#include <mart-common/StringView.h>

#include <catch2/catch.hpp>

#include <string>

TEST_CASE( "various_string_view", "[StringView]" )
{
	CHECK( "Hello" == mart::StringView( "Hello" ) );
	CHECK( "Hello" == mart::StringView( "Hello" ).to_string() );
	CHECK( "Hello" == mart::StringView( "1 Hello 123" ).substr( 2, 5 ) );
	CHECK( "Hello 123" == mart::StringView( "1 Hello 123" ).substr( 2 ) );
	CHECK( "Hello " != mart::StringView( "Hello" ) );
}

TEST_CASE( "string_subview", "[StringView]" )
{
	CHECK_THROWS( mart::StringView( "Hello" ).substr( 10 ) );
	CHECK_THROWS( mart::StringView( "Hello" ).substr( 3, 10 ) );
	CHECK_THROWS( mart::StringView( "Hello" ).substr( 10, 0 ) );
	CHECK( "Hello" == mart::StringView( "Hello" ).substr( 0, 5 ) );
	CHECK( "" == mart::StringView( "Hello" ).substr( 5 ) );
	CHECK( "" == mart::StringView( "Hello" ).substr( 5, 0 ) );
	CHECK( "" == mart::StringView( "Hello" ).substr( 0, 0 ) );
}

TEST_CASE( "to_integral", "[StringView]" )
{
	for( int i = -1000; i < 10000; ++i ) {
		CHECK( i == mart::to_integral( std::to_string( i ) + "  " ) );
	}
	for( unsigned int i = 0; i < 10000; ++i ) {
		CHECK( i == mart::to_integral_unsafe( std::to_string( i ) ) );
	}
}
