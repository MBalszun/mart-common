#include <mart-common/PrintWrappers.h>

#include <catch2/catch.hpp>

#include <type_traits>

#include <array>
#include <numeric>
#include <sstream>

TEST_CASE( "Padded", "[PrintWrappers]" )
{
	std::stringstream ss;

	ss << mart::padded( "abcd", 8, mart::Pad::Left );
	CHECK( ss.str() == "    abcd" );
	ss.str( "" );

	ss << mart::padded( "abcd", 8, mart::Pad::Middle );
	CHECK( ss.str() == "  abcd  " );
	ss.str( "" );

	ss << mart::padded( "abcd", 8, mart::Pad::Right );
	CHECK( ss.str() == "abcd    " );
	ss.str( "" );

	for( int i = 0; i < 8; ++i ) {
		ss << mart::padded( "abcd", i, mart::Pad::Left );
		CHECK( ss.str().size() == std::max( 4, i ) );
		ss.str( "" );
	}
}

TEST_CASE( "Memory view", "[PrintWrappers]" )
{
	std::array<int, 40> data{};
	std::iota( data.begin(), data.end(), 0 );

	std::stringstream ss;
	ss << mart::sformat( mart::view_bytes( data ) );

	CHECK( true );
}
