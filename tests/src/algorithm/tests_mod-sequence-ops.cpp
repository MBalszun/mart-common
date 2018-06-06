#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <iterator>

#include "./testranges.h"

TEST_CASE( "copy_compare_mart_output_to_std_output", "[algorithm][copy]" )
{
	for( auto&& rng : test_ranges ) {
		std::vector<int> dest_mart;
		std::vector<int> dest_std;

		mart::copy( rng, mart::back_inserter_range( dest_mart ) );
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

		mart::move( src_mart, mart::back_inserter_range( dest_mart ) );
		std::move( src_std.begin(), src_std.end(), std::back_inserter( dest_std ) );
		CHECK( dest_mart == dest_std );
	}

	for( auto&& rng : test_ranges ) {
		std::vector<int> src_mart( rng );
		std::vector<int> src_std( rng );
		std::vector<int> dest_mart( src_mart.size() );
		std::vector<int> dest_std( src_std.size() );

		auto rit_mart = mart::move( src_mart, dest_mart );
		auto rit_std  = std::move( src_std.begin(), src_std.end(), dest_std.begin() );
		CHECK( rit_mart.begin() - dest_mart.begin() == rit_std - dest_std.begin() );
		CHECK( dest_mart == dest_std );
	}
}

TEST_CASE( "copy_if_compare_mart_output_to_std_output", "[algorithm][copy_if]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto pred : unary_preds ) {
			std::vector<int> dest_mart;
			std::vector<int> dest_std;

			mart::copy_if( rng, mart::back_inserter_range( dest_mart ), pred );
			std::copy_if( rng.begin(), rng.end(), std::back_inserter( dest_std ), pred );
			CHECK( dest_mart == dest_std );
		}
	}

	for( auto&& rng : test_ranges ) {
		for( auto pred : unary_preds ) {
			std::vector<int> dest_mart( rng.size() );
			std::vector<int> dest_std( rng.size() );

			auto r_mart = mart::copy_if( rng, dest_mart, pred );
			auto r_std = std::copy_if( rng.begin(), rng.end(), dest_std.begin(), pred );
			auto dist_mart = r_mart.begin() - dest_mart.begin();
			auto dist_std  = r_std - dest_std.begin();
			CHECK( dist_mart == dist_std );
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
		for( int i = -5; i < 5; i++ ) {}
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

TEST_CASE( "transform_SISO_compare_mart_output_to_std_output", "[algorithm][transform]" )
{
	for( auto&& rng : test_ranges ) {
		for( auto&& trans : siso_trafos ) {
			std::vector<int> dest_mart( rng.size() );
			std::vector<int> dest_std( rng.size() );

			std::transform( rng.begin(), rng.end(), dest_std.begin(), trans );
			mart::transform( rng, dest_mart, trans );
			CHECK( dest_mart == dest_std );
		}
	}
}

TEST_CASE( "transform_MISO_compare_mart_output_to_std_output", "[algorithm][transform]" )
{
	for( auto&& rng1 : test_ranges ) {
		for( auto&& rng2 : test_ranges ) {
			if( rng1.size() != rng2.size() ) { continue; }
			for( auto&& trans : miso_trafos ) {
				std::vector<int> dest_mart( rng1.size() );
				std::vector<int> dest_std( rng1.size() );

				std::transform( rng1.begin(), rng1.end(), rng2.begin(), dest_std.begin(), trans );
				mart::transform( rng1, rng2, dest_mart, trans );
				CHECK( dest_mart == dest_std );
			}
		}
	}
}
