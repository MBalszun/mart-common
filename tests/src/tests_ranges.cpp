#include <mart-common/ranges.h>

#include <catch2/catch.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <vector>

TEST_CASE( "irange_produces_integer_values_from_min_inclusive_to_max_exclusive", "[ranges][irange]" )
{
	const std::vector<int> expected_values {-1, 0, 1, 2, 3, 4};
	std::vector<int>       values;

	for( auto i : mart::irange( -1, 5 ) ) {
		values.push_back( i );
	}
	CHECK( std::equal( values.begin(), values.end(), expected_values.begin(), expected_values.end() ) );
}

TEST_CASE( "irange_produces_integer_values_from_0_to_max_exclusive", "[ranges][irange]" )
{
	const std::vector<int> expected_values {0, 1, 2, 3, 4};
	std::vector<int>       values;

	for( auto i : mart::irange( 5 ) ) {
		values.push_back( i );
	}
	CHECK( std::equal( values.begin(), values.end(), expected_values.begin(), expected_values.end() ) );
}

TEST_CASE( "irange_produces_all_indexes_for_vector", "[ranges][irange]" )
{
	std::vector<bool> visited_idx( 20, false );

	for( auto i : mart::irange( visited_idx ) ) {
		visited_idx[i] = true;
	}
	CHECK( std::all_of( visited_idx.begin(), visited_idx.end(), []( bool e ) { return e; } ) );
}

TEST_CASE( "irange_produces_all_indexes_for_std_array", "[ranges][irange]" )
{
	std::array<bool, 20> visited_idx {};

	for( auto i : mart::irange( visited_idx ) ) {
		visited_idx[i] = true;
	}
	CHECK( std::all_of( visited_idx.begin(), visited_idx.end(), []( bool e ) { return e; } ) );
}

TEST_CASE( "irange_produces_all_indexes_for_c_array", "[ranges][irange]" )
{
	bool visited_idx[20] {};

	for( auto i : mart::irange( visited_idx ) ) {
		visited_idx[i] = true;
	}
	CHECK( std::all_of( std::begin( visited_idx ), std::end( visited_idx ), []( bool e ) { return e; } ) );
}

TEST_CASE( "irange_empty_ranges_produces_no_values", "[ranges][irange]" )
{
	std::vector<int> values;
	for( auto i : mart::irange( -1, -1 ) ) {
		values.push_back( i );
	}
	for( auto i : mart::irange( 0, 0 ) ) {
		values.push_back( i );
	}
	for( auto i : mart::irange( 10, 10 ) ) {
		values.push_back( i );
	}
	CHECK( values.empty() );
}
