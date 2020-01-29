#include <mart-common/ArrayView.h>

#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <vector>

TEST_CASE( "constructed_from_container_has_same_elements", "[ArrayView]" )
{
	int na1[] = {-3};
	int na2[] = {-3, 1, 5, 6, 7, 8};

	std::array<int, 1>  a1 = {-10};
	std::array<int, 10> a2 = {-15, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	std::vector<int> v1{};
	std::vector<int> v2{-10};
	std::vector<int> v3{-15, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	mart::ArrayView<int> view_na1( na1 );
	mart::ArrayView<int> view_na2( na2 );

	mart::ArrayView<int> view_a1( a1 );
	mart::ArrayView<int> view_a2( a2 );

	mart::ArrayView<int> view_v1( v1 );
	mart::ArrayView<int> view_v2( v2 );
	mart::ArrayView<int> view_v3( v3 );

	CHECK( std::equal( std::begin( na1 ), std::end( na1 ), view_na1.begin(), view_na1.end() ) );
	CHECK( std::equal( std::begin( na2 ), std::end( na2 ), view_na2.begin(), view_na2.end() ) );

	CHECK( std::equal( a1.begin(), a1.end(), view_a1.begin(), view_a1.end() ) );
	CHECK( std::equal( a2.begin(), a2.end(), view_a2.begin(), view_a2.end() ) );

	CHECK( std::equal( v1.begin(), v1.end(), view_v1.begin(), view_v1.end() ) );
	CHECK( std::equal( v2.begin(), v2.end(), view_v2.begin(), view_v2.end() ) );
	CHECK( std::equal( v3.begin(), v3.end(), view_v3.begin(), view_v3.end() ) );
}

TEST_CASE( "array_view_subview_throws_when_invalid_range_is_specified", "[ArrayView]" )
{
	int na[] = {-3, 1, 5, 6, 7, 8};

	mart::ArrayView<int> view_na( na );

	bool exception_thrown = false;

	try {

		[[maybe_unused]] auto sr = view_na.subview( 0, 100 );

	} catch( ... ) {
		exception_thrown = true;
	}

	CHECK( exception_thrown );
}

TEST_CASE( "array_view_copy_works_and_doesnt_collide_with_general_std_copy_wrapper", "[ArrayView]" )
{
	const std::vector<int> src = {-3, 1, 5, 6, 7, 8};
	std::vector<int>       dest( src.size() + 5 );

	mart::ArrayView<const int> view_src( src );
	mart::ArrayView<int>       view_dest( dest );

	auto rem = mart::copy( view_src, view_dest );

	CHECK( mart::equal( view_src, view_dest.subview( 0, view_src.size() ) ) );
	CHECK( rem.size() == dest.size() - src.size() );
}

TEST_CASE( "asBytes", "[ArrayView]" )
{
	int       i{1};
	const int ci{2};

	auto crange1 = mart::view_bytes( i );
	auto crange2 = mart::view_bytes_const( i );
	auto crange3 = mart::view_bytes( ci );
	auto range1  = mart::view_bytes_mutable( i );

	static_assert( std::is_same_v<decltype( crange1 ), mart::ConstMemoryView> );
	static_assert( std::is_same_v<decltype( crange2 ), mart::ConstMemoryView> );
	static_assert( std::is_same_v<decltype( crange3 ), mart::ConstMemoryView> );
	static_assert( std::is_same_v<decltype( range1 ), mart::MemoryView> );

	CHECK( crange1.size() == sizeof( ci ) );
	CHECK( crange2.size() == sizeof( ci ) );
	CHECK( crange3.size() == sizeof( ci ) );
	CHECK( range1.size() == sizeof( i ) );
}
