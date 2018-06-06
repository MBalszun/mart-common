#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <array>

#include "./testranges.h"

TEST_CASE( "find_returns_iterator_to_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find( data, 2 );
	CHECK( it == data.begin() + 1 );
}

TEST_CASE( "find_returns_end_iterator_if_element_doesnt_exist", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find( data, 0 );
	CHECK( it == data.end() );
}

TEST_CASE( "find_returns_end_iterator_on_empty_range", "[algorithm][find]" )
{
	std::array<int, 0> data{};
	auto			   it = mart::find( data, 0 );
	CHECK( it == data.end() );
}

TEST_CASE( "find_finds_first_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find( data, 1 );
	CHECK( it == data.begin() );
}

TEST_CASE( "find_finds_last_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find( data, 5 );
	CHECK( it == data.end() - 1 );
}

/* find_if*/
TEST_CASE( "find_if_returns_iterator_to_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if( data, []( auto e ) { return e == 2; } );
	CHECK( it == data.begin() + 1 );
}

TEST_CASE( "find_if_returns_end_iterator_if_element_doesnt_exist", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if( data, []( auto e ) { return e == 0; } );
	CHECK( it == data.end() );
}

TEST_CASE( "find_if_returns_end_iterator_on_empty_range", "[algorithm][find]" )
{
	std::array<int, 0> data{};
	auto			   it = mart::find_if( data, []( auto e ) { return e == 0; } );
	CHECK( it == data.end() );
}

TEST_CASE( "find_if_finds_first_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if( data, []( auto e ) { return e == 1; } );
	CHECK( it == data.begin() );
}

TEST_CASE( "find_if_finds_last_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if( data, []( auto e ) { return e == 5; } );
	CHECK( it == data.end() - 1 );
}

/* find_if_ex*/
TEST_CASE( "find_if_ex_returns_iterator_to_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if_ex( data, []( auto e ) { return e == 2; } );
	CHECK( it );
	CHECK( it == data.begin() + 1 );
}

TEST_CASE( "find_if_ex_returns_end_iterator_if_element_doesnt_exist", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if_ex( data, []( auto e ) { return e == 0; } );
	CHECK( !it );
	CHECK( it == data.end() );
}

TEST_CASE( "find_if_ex_returns_end_iterator_on_empty_range", "[algorithm][find]" )
{
	std::array<int, 0> data{};
	auto			   it = mart::find_if_ex( data, []( auto e ) { return e == 0; } );
	CHECK( !it );
	CHECK( it == data.end() );
}

TEST_CASE( "find_if_ex_finds_first_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if_ex( data, []( auto e ) { return e == 1; } );
	CHECK( it );
	CHECK( it == data.begin() );
}

TEST_CASE( "find_if_ex_finds_last_element", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	auto			   it = mart::find_if_ex( data, []( auto e ) { return e == 5; } );
	CHECK( it );
	CHECK( it == data.end() - 1 );
}

/* find_last_if*/

TEST_CASE( "find_last_if_finds_last_matching_element", "[algorithm][find]" )
{
	std::array<int, 5> data{5, 2, 5, 5, 4};

	auto it = mart::find_last_if( data, []( auto e ) { return e == 5; } );
	CHECK( it == data.begin() + 3 );
}

TEST_CASE( "find_last_if_returns_end_when_no_match", "[algorithm][find]" )
{
	std::array<int, 5> data{1, 2, 3, 3, 4};

	auto it = mart::find_last_if( data, []( auto e ) { return e == 5; } );
	CHECK( it == data.end() );
}

TEST_CASE( "find_last_if_returns_fist_element", "[algorithm][find]" )
{
	std::array<int, 5> data{5, 2, 3, 3, 4};

	auto it = mart::find_last_if( data, []( auto e ) { return e == 5; } );
	CHECK( it == data.begin() );
}

TEST_CASE( "find_end_returns_fist_element", "[algorithm][find]" )
{
	std::array<int, 5> data{5, 2, 3, 3, 4};

	auto it = mart::find_last_if( data, []( auto e ) { return e == 5; } );
	CHECK( it == data.begin() );
}

TEST_CASE( "find_compare_mart_output_to_std_output", "[algorithm][find]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			for( auto v : rng2 ) {
				CHECK( mart::find( rng, v ) == std::find( rng.begin(), rng.end(), v ) );
			}
		}
	}
}

TEST_CASE( "find_if_compare_mart_output_to_std_output", "[algorithm][find_if]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& p : unary_preds ) {
			CHECK( mart::find_if( rng, p ) == std::find_if( rng.begin(), rng.end(), p ) );
		}
	}
}

TEST_CASE( "find_if_not_compare_mart_output_to_std_output", "[algorithm][find_if_not]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& p : unary_preds ) {
			CHECK( mart::find_if_not( rng, p ) == std::find_if_not( rng.begin(), rng.end(), p ) );
		}
	}
}

TEST_CASE( "find_if_end_compare_mart_output_to_std_output", "[algorithm][find_if_end]" )
{
	for( auto&& rng1 : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			CHECK( mart::find_end( rng1, rng2 )
				   == std::find_end( rng1.begin(), rng1.end(), rng2.begin(), rng2.end() ) );
		}
	}
}

TEST_CASE( "find_first_of_compare_mart_output_to_std_output", "[algorithm][find_first_of]" )
{
	for( auto&& rng1 : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			CHECK( mart::find_first_of( rng1, rng2 )
				   == std::find_first_of( rng1.begin(), rng1.end(), rng2.begin(), rng2.end() ) );
			for( auto&& p : binary_preds ) {
				CHECK( mart::find_first_of( rng1, rng2, p )
					   == std::find_first_of( rng1.begin(), rng1.end(), rng2.begin(), rng2.end(), p ) );
			}
		}
	}
}

TEST_CASE( "adjacent_find_compare_mart_output_to_std_output", "[algorithm][adjacent_find]" )
{
	for( auto&& rng1 : test_ranges ) {
		CHECK( mart::adjacent_find( rng1 ) == std::adjacent_find( rng1.begin(), rng1.end() ) );
	}
}

TEST_CASE( "lower_bound_of_compare_mart_output_to_std_output", "[algorithm][lower_bound]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& vrng : test_ranges ) {
			for( auto v : vrng ) {
				CHECK( mart::lower_bound( rng, v ) == std::lower_bound( rng.begin(), rng.end(), v ) );
				for( auto&& c : comps ) {
					CHECK( mart::lower_bound( rng, v, c )
						   == std::lower_bound( rng.begin(), rng.end(), v, c ) );
				}
			}
		}
	}
}

TEST_CASE( "upper_bound_of_compare_mart_output_to_std_output", "[algorithm][upper_bound]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& vrng : test_ranges ) {
			for( auto v : vrng ) {
				CHECK( mart::upper_bound( rng, v ) == std::upper_bound( rng.begin(), rng.end(), v ) );
				for( auto&& c : comps ) {
					CHECK( mart::upper_bound( rng, v, c ) == std::upper_bound( rng.begin(), rng.end(), v, c ) );
				}
			}
		}
	}
}
