#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <iterator>

#include "./testranges.h"

TEST_CASE( "copy_compare_mart_output_to_std_output", "[algorithm][copy]" )
{
	for( auto&& rng : test_ranges ) {
		std::vector<int> dest_mart;
		std::vector<int> dest_std;

		mart::copy( rng, std::back_inserter( dest_mart ) );
		std::copy( rng.begin(), rng.end(), std::back_inserter( dest_std ) );
		CHECK( dest_mart == dest_std );
	}
}

TEST_CASE( "move_compare_mart_output_to_std_output", "[algorithm][move]" )
{
	for( auto&& rng : test_ranges ) {
		std::vector<int> src_mart( rng );
		std::vector<int> src_std( rng );
		std::vector<int> dest_mart;
		std::vector<int> dest_std;

		mart::move( src_mart, std::back_inserter( dest_mart ) );
		std::move( src_std.begin(), src_std.end(), std::back_inserter( dest_std ) );
		CHECK( dest_mart == dest_std );
	}
}

TEST_CASE( "copy_if_compare_mart_output_to_std_output", "[algorithm][copy_if]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto pred : unary_preds ) {
			std::vector<int> dest_mart;
			std::vector<int> dest_std;

			mart::copy_if( rng, std::back_inserter( dest_mart ), pred );
			std::copy_if( rng.begin(), rng.end(), std::back_inserter( dest_std ), pred );
			CHECK( dest_mart == dest_std );
		}
	}
}

TEST_CASE( "fill_compare_mart_output_to_std_output", "[algorithm][fill]" )
{
	for( int i = -5; i < 5; i++ ) {
		std::vector<int> rng_mart( 10 );
		std::vector<int> rng_std( 10 );
		mart::fill( rng_mart, i );
		std::fill( rng_std.begin(), rng_std.end(), i );
		CHECK( rng_mart == rng_std );
	}
}

TEST_CASE( "fill_n_compare_mart_output_to_std_output", "[algorithm][fill_n]" )
{
	for( int v = -5; v < 5; v++ ) {
		std::vector<int> rng_mart( 10 );
		std::vector<int> rng_std( 10 );
		for( auto i = 0u; i < rng_mart.size(); ++i ) {
			mart::fill_n( rng_mart, i, v );
			std::fill_n( rng_std.begin(), i, v );
			CHECK( rng_mart == rng_std );
		}
	}
}

TEST_CASE( "generate_compare_mart_output_to_std_output", "[algorithm][generate]" )
{

	for( auto&& gen : generators ) {
		for( int i = -5; i < 5; i++ ) {
		}
		std::vector<int> rng_mart( 10 );
		std::vector<int> rng_std( 10 );
		mart::generate( rng_mart, gen );
		std::generate( rng_std.begin(), rng_std.end(), gen );
		CHECK( rng_mart == rng_std );
	}
}

TEST_CASE( "generate_n_compare_mart_output_to_std_output", "[algorithm][generate_n]" )
{
	for( auto&& gen : generators ) {
		std::vector<int> rng_mart( 10 );
		std::vector<int> rng_std( 10 );
		for( auto i = 0u; i < rng_mart.size(); ++i ) {
			mart::generate_n( rng_mart, i, gen );
			std::generate_n( rng_std.begin(), i, gen );
			CHECK( rng_mart == rng_std );

		}
	}
}
