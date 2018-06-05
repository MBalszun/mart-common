#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <iterator>

#include "./testranges.h"

TEST_CASE( "copy_compare_mart_output_to_std_output", "[algoriths][copy]" )
{
	for( auto&& rng : test_ranges ) {
		std::vector<int> dest_mart;
		std::vector<int> dest_std;

		mart::copy( rng, std::back_inserter( dest_mart ) );
		std::copy( rng.begin(), rng.end(), std::back_inserter( dest_std ) );
		CHECK( dest_mart == dest_std );
	}
}

TEST_CASE( "move_compare_mart_output_to_std_output", "[algoriths][move]" )
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

TEST_CASE( "copy_if_compare_mart_output_to_std_output", "[algoriths][copy_if]" )
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
