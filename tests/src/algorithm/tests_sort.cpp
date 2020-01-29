#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <functional>
#include <vector>

#include "./testranges.h"
#include <mart-common/cpp_std/execution.h>

TEST_CASE( "algo_sort_range_is_sorted_after_sorting", "[algorithm][sort]" )
{
	std::vector<int> v {94, -1, 5, 3, 66, 3};

	mart::sort( v );
	CHECK( mart::is_sorted( v ) );
}

TEST_CASE( "algo_sort_range_is_reverse_sorted_after_reverse_sorting", "[algorithm][sort]" )
{
	std::vector<int> v {94, -1, 5, 3, 66, 3};

	mart::sort( v, std::greater<> {} );
	CHECK( mart::is_sorted( v, std::greater<> {} ) );
}

TEST_CASE( "algo_sort_range_is_sorted_after_parallel_sorting", "[algorithm][sort][mt]" )
{
	std::vector<int> v {94, -1, 5, 3, 66, 3};

	mart::sort( mart::execution::par, v );
	CHECK( mart::is_sorted( v ) );
}

TEST_CASE( "algo_sort_range_is_reverse_sorted_after_parallel_reverse_sorting", "[algorithm][sort][mt]" )
{
	std::vector<int> v {94, -1, 5, 3, 66, 3};

	mart::sort( mart::execution::par, v, std::greater<> {} );
	CHECK( mart::is_sorted( v, std::greater<> {} ) );
}

TEST_CASE( "sort_compare_mart_output_to_std_output", "[algorithm][sort]" )
{
	for( auto&& rng1 : test_ranges ) {
		auto cpy_mart = rng1;
		auto cpy_std  = rng1;

		CHECK( mart::is_sorted( cpy_mart ) == std::is_sorted( cpy_std.begin(), cpy_std.end() ) );
		CHECK( mart::is_sorted_until( cpy_std ) == std::is_sorted_until( cpy_std.begin(), cpy_std.end() ) );

		mart::sort( cpy_mart );
		std::sort( cpy_std.begin(), cpy_std.end() );

		CHECK( cpy_mart == cpy_std );
		CHECK( mart::is_sorted( cpy_mart ) == std::is_sorted( cpy_std.begin(), cpy_std.end() ) );
		for( auto cmp : comps ) {

			CHECK( mart::is_sorted_until( cpy_mart, cmp ) - cpy_mart.begin()
				   == std::is_sorted_until( cpy_std.begin(), cpy_std.end(), cmp ) - cpy_std.begin() );

			mart::sort( cpy_mart, cmp );
			std::sort( cpy_std.begin(), cpy_std.end(), cmp );

			CHECK( cpy_mart == cpy_std );
			CHECK( mart::is_sorted( cpy_mart, cmp ) == std::is_sorted( cpy_std.begin(), cpy_std.end(), cmp ) );
		}
	}
}

TEST_CASE( "stable_sort_compare_mart_output_to_std_output", "[algorithm][stable_sort]" )
{
	for( auto&& rng1 : test_ranges ) {
		auto cpy_mart = rng1;
		auto cpy_std  = rng1;

		mart::stable_sort( cpy_mart );
		std::stable_sort( cpy_std.begin(), cpy_std.end() );

		CHECK( cpy_mart == cpy_std );

		for( auto cmp : comps ) {

			mart::stable_sort( cpy_mart, cmp );
			std::stable_sort( cpy_std.begin(), cpy_std.end(), cmp );

			CHECK( cpy_mart == cpy_std );
		}
	}
}

TEST_CASE( "partial_sort_compare_mart_output_to_std_output", "[algorithm][partial_sort]" )
{
	for( auto&& rng1 : test_ranges ) {
		auto cpy_mart = rng1;
		auto cpy_std  = rng1;

		for( auto i = 0u; i < rng1.size(); ++i ) {
			mart::partial_sort( cpy_mart, cpy_mart.begin() + i );
			std::partial_sort( cpy_std.begin(), cpy_std.begin() + i, cpy_std.end() );
			CHECK( cpy_mart == cpy_std );
		}

		for( auto cmp : comps ) {
			for( auto i = 0u; i < rng1.size(); ++i ) {

				mart::partial_sort( cpy_mart, cpy_mart.begin() + i, cmp );
				std::partial_sort( cpy_std.begin(), cpy_std.begin() + i, cpy_std.end(), cmp );

				CHECK( cpy_mart == cpy_std );
			}
		}
	}
}

TEST_CASE( "partial_sort_copy_compare_mart_output_to_std_output", "[algorithm][partial_sort_copy]" )
{
	for( auto&& rng : test_ranges ) {

		for( auto i = 0u; i < rng.size() * 2; ++i ) {
			std::vector<int> dest_mart( i );
			std::vector<int> dest_std( i );

			mart::partial_sort_copy( rng, dest_mart );
			std::partial_sort_copy( rng.begin(), rng.end(), dest_std.begin(), dest_std.end() );

			CHECK( dest_mart == dest_std );
		}

		for( auto cmp : comps ) {
			for( auto i = 0u; i < rng.size(); ++i ) {

				std::vector<int> dest_mart( i );
				std::vector<int> dest_std( i );

				mart::partial_sort_copy( rng, dest_mart, cmp );
				std::partial_sort_copy( rng.begin(), rng.end(), dest_std.begin(), dest_std.end(), cmp );

				CHECK( dest_mart == dest_std );
			}
		}
	}
}

TEST_CASE( "nth_element_compare_mart_output_to_std_output", "[algorithm][nth_element]" )
{
	for( auto&& rng1 : test_ranges ) {
		auto cpy_mart = rng1;
		auto cpy_std  = rng1;

		for( auto i = 0u; i < rng1.size(); ++i ) {
			mart::nth_element( cpy_mart, cpy_mart.begin() + i );
			std::nth_element( cpy_std.begin(), cpy_std.begin() + i, cpy_std.end() );
			CHECK( cpy_mart == cpy_std );
		}

		for( auto cmp : comps ) {
			for( auto i = 0u; i < rng1.size(); ++i ) {
				mart::nth_element( cpy_mart, cpy_mart.begin() + i, cmp );
				std::nth_element( cpy_std.begin(), cpy_std.begin() + i, cpy_std.end(), cmp );
				CHECK( cpy_mart == cpy_std );
			}
		}
	}
}
