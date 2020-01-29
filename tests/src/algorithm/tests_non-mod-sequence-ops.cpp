#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include "./testranges.h"

TEST_CASE( "none_of_compare_mart_output_to_std_output", "[algorithm][none_of]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& p : unary_preds ) {
			CHECK( mart::none_of( rng, p ) == std::none_of( rng.begin(), rng.end(), p ) );
		}
	}
}

TEST_CASE( "any_of_compare_mart_output_to_std_output", "[algorithm][any_of]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& p : unary_preds ) {
			CHECK( mart::any_of( rng, p ) == std::any_of( rng.begin(), rng.end(), p ) );
		}
	}
}

TEST_CASE( "all_of_compare_mart_output_to_std_output", "[algorithm][all_of]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& p : unary_preds ) {
			CHECK( mart::all_of( rng, p ) == std::all_of( rng.begin(), rng.end(), p ) );
		}
	}
}

TEST_CASE( "count_if_compare_mart_output_to_std_output", "[algorithm][count_if]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& p : unary_preds ) {
			CHECK( mart::count_if( rng, p ) == std::count_if( rng.begin(), rng.end(), p ) );
		}
	}
}

TEST_CASE( "count_compare_mart_output_to_std_output", "[algorithm][count]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			for( auto v : rng2 ) {
				CHECK( mart::count( rng, v ) == std::count( rng.begin(), rng.end(), v ) );
			}
		}
	}
}

TEST_CASE( "search_compare_mart_output_to_std_output", "[algorithm][search]" )
{
	for( auto&& rng1 : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			CHECK( mart::search( rng1, rng2 ) == std::search( rng1.begin(), rng1.end(), rng2.begin(), rng2.end() ) );
			for( auto&& p : binary_preds ) {
				CHECK( mart::search( rng1, rng2, p )
					   == std::search( rng1.begin(), rng1.end(), rng2.begin(), rng2.end(), p ) );
			}
		}
	}
}

TEST_CASE( "search_n_compare_mart_output_to_std_output", "[algorithm][search]" )
{
	for( auto&& rng1 : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			for( auto v : rng2 ) {
				for( auto cnt : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10} ) {

					CHECK( mart::search_n( rng1, cnt, v ) == std::search_n( rng1.begin(), rng1.end(), cnt, v ) );
					for( auto&& p : binary_preds ) {
						CHECK( mart::search_n( rng1, cnt, v, p )
							   == std::search_n( rng1.begin(), rng1.end(), cnt, v, p ) );
					}
				}
			}
		}
	}
}

TEST_CASE( "for_each_compare_mart_output_to_std_output", "[algorithm][search]" )
{
	for( auto&& rng1 : test_ranges ) {
		std::vector<int> accum_mart;
		std::vector<int> accum_std;
		mart::for_each( rng1, [&]( int i ) { accum_mart.push_back( i ); } );
		std::for_each( rng1.begin(), rng1.end(), [&]( int i ) { accum_std.push_back( i ); } );
		CHECK( accum_mart == accum_std );
	}
}

TEST_CASE( "mismatch_compare_mart_output_to_std_output", "[algorithm][search]" )
{
	for( auto&& rng1 : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			CHECK( mart::mismatch( rng1, rng2 )
				   == std::mismatch( rng1.begin(), rng1.end(), rng2.begin(), rng2.end() ) );
			for( auto&& p : binary_preds ) {
				CHECK( mart::mismatch( rng1, rng2, p )
					   == std::mismatch( rng1.begin(), rng1.end(), rng2.begin(), rng2.end(), p ) );
			}
		}
	}
}
