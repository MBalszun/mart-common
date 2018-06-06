#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include "./testranges.h"

TEST_CASE( "set_union_compare_mart_output_to_std_output", "[algorithm][set_union]" )
{
	for( auto&& r1 : test_ranges ) {
		for( auto&& r2 : test_ranges ) {
			auto rng1 = r1;
			auto rng2 = r2;
			mart::sort( rng1 );
			mart::sort( rng2 );

			std::vector<int> dest_std;
			std::vector<int> dest_mart;

			std::set_union( rng1.begin(), rng1.end(), rng2.begin(), rng2.end(), std::back_inserter( dest_std ) );
			mart::set_union( rng1, rng2, std::back_inserter( dest_mart ) );
			CHECK( dest_mart == dest_std );
		}
	}
}

TEST_CASE( "set_difference_compare_mart_output_to_std_output", "[algorithm][set_difference]" )
{
	for( auto&& r1 : test_ranges ) {
		for( auto&& r2 : test_ranges ) {
			auto rng1 = r1;
			auto rng2 = r2;
			mart::sort( rng1 );
			mart::sort( rng2 );

			std::vector<int> dest_std;
			std::vector<int> dest_mart;

			std::set_difference( rng1.begin(), rng1.end(), rng2.begin(), rng2.end(), std::back_inserter( dest_std ) );
			mart::set_difference( rng1, rng2, std::back_inserter( dest_mart ) );
			CHECK( dest_mart == dest_std );
		}
	}
}