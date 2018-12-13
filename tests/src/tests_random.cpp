#include <mart-common/random.h>

#include <catch2/catch.hpp>

TEST_CASE( "Random_values", "[random]" )
{

	for( int i = 0; i < 100; ++i ) {
		CHECK( mart::getRandomBool( 1.0 ) );
	}
	for( int i = 0; i < 100; ++i ) {
		CHECK( mart::getRandomBool( 0.0 ) == false );
	}

	for( int i = 0; i < 100; ++i ) {
		int v = mart::getRandomInt( -5, 7 );
		CHECK( v <= 7 );
		CHECK( v >= -5 );
	}
}