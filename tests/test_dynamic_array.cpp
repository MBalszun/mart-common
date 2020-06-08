#include <im_str/detail/dynamic_array.hpp>

#include "include_catch.hpp"

#include <vector>

TEST_CASE( "dynamic_array_list_constructor_simple", "[im_str]" )
{
	mba::detail::dynamic_array<std::size_t>       arr{ 0, 1, 2, 3, 4, 5, 6 };
	const mba::detail::dynamic_array<std::size_t> carr{ 0, 1, 2, 3, 4, 5, 6 };

	CHECK( arr.size() == 7 );
	CHECK( carr.size() == 7 );

	for( std::size_t i = 0; i < arr.size(); ++i ) {
		CHECK( arr[i] == i );
	}
	for( std::size_t i = 0; i < carr.size(); ++i ) {
		CHECK( carr[i] == i );
	}

	std::size_t cnt = 0;
	for( auto i : arr ) {
		CHECK( i == cnt );
		cnt++;
	}

	cnt = 0;
	for( auto i : carr ) {
		CHECK( i == cnt );
		cnt++;
	}
}

TEST_CASE( "dynamic_array_list_constructor_complex", "[im_str]" )
{
	mba::detail::dynamic_array<std::vector<std::size_t>>       arr{ { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 } };
	const mba::detail::dynamic_array<std::vector<std::size_t>> carr{ { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 } };

	CHECK( arr.size() == 4 );
	CHECK( carr.size() == 4 );

	for( std::size_t i = 0; i < arr.size(); ++i ) {
		auto ref = std::vector<std::size_t>{ i * 2, i * 2 + 1 };
		CHECK( arr[i] == ref );
	}
	for( std::size_t i = 0; i < carr.size(); ++i ) {
		auto ref = std::vector<std::size_t>{ i * 2, i * 2 + 1 };
		CHECK( carr[i] == ref );
	}

	std::size_t cnt = 0;
	for( auto e : arr ) {
		CHECK( e == std::vector<std::size_t>{ cnt, cnt + 1 } );
		cnt += 2;
	}

	cnt = 0;
	for( auto e : arr ) {
		CHECK( e == std::vector<std::size_t>{ cnt, cnt + 1 } );
		cnt += 2;
	}
}

namespace {

void check_array( const mba::detail::dynamic_array<std::vector<int>>& arr )
{
	CHECK( arr.size() == 5 );
	int cnt = 0;
	for( const auto& e : arr ) {
		CHECK( e.size() == (std::size_t)cnt );
		for( int i = 0; i < cnt; ++i ) {
			CHECK( e[i] == i );
		}
		cnt++;
	}
}

mba::detail::dynamic_array<std::vector<int>> make_array()
{
	mba::detail::dynamic_array<std::vector<int>> arr( 5 );
	CHECK( arr.size() == 5 );
	int cnt = 0;
	for( auto& e : arr ) {
		for( int i = 0; i < cnt; ++i ) {
			e.push_back( i );
		}
		cnt++;
	}
	check_array( arr );
	return arr;
}

} // namespace

TEST_CASE( "dynamic_array_other_constructors_complex", "[im_str]" )
{
	mba::detail::dynamic_array<std::vector<int>> arr = make_array();
	check_array( arr );

	const auto carr( arr );
	check_array( carr );

	// check copy construct
	mba::detail::dynamic_array<std::vector<int>> arr01( arr );
	mba::detail::dynamic_array<std::vector<int>> arr02( arr );
	mba::detail::dynamic_array<std::vector<int>> arr03( arr );

	check_array( arr01 );
	check_array( arr02 );
	check_array( arr03 );

	// check move construct
	mba::detail::dynamic_array<std::vector<int>> arr11( std::move( arr01 ) );
	mba::detail::dynamic_array<std::vector<int>> arr12( std::move( arr02 ) );
	mba::detail::dynamic_array<std::vector<int>> arr13( std::move( arr03 ) );

	check_array( arr11 );
	check_array( arr12 );
	check_array( arr13 );

	// check copy assign
	mba::detail::dynamic_array<std::vector<int>> arr21;
	mba::detail::dynamic_array<std::vector<int>> arr22( 5 );
	mba::detail::dynamic_array<std::vector<int>> arr23( 20 );

	arr21 = arr;
	arr22 = arr;
	arr23 = arr;

	check_array( arr21 );
	check_array( arr22 );
	check_array( arr23 );

	// check move assign
	mba::detail::dynamic_array<std::vector<int>> arr31;
	mba::detail::dynamic_array<std::vector<int>> arr32( 5 );
	mba::detail::dynamic_array<std::vector<int>> arr33( 20 );

	arr31 = std::move( arr11 );
	arr32 = std::move( arr12 );
	arr33 = std::move( arr13 );

	check_array( arr31 );
	check_array( arr32 );
	check_array( arr33 );
}
