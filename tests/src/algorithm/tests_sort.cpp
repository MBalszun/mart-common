#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <functional>
#include <vector>

TEST_CASE( "algo_sort_range_is_sorted_after_sorting", "[algorithm][sort]" )
{
	std::vector<int> v{94, -1, 5, 3, 66, 3};

	mart::sort( v );
	CHECK( mart::is_sorted( v ) );
}

TEST_CASE( "algo_sort_range_is_reverse_sorted_after_reverse_sorting", "[algorithm][sort]" )
{
	std::vector<int> v{94, -1, 5, 3, 66, 3};

	mart::sort( v, std::greater<>{} );
	CHECK( mart::is_sorted( v, std::greater<>{} ) );
}

#ifdef _MSC_VER

#include <execution>

TEST_CASE( "algo_sort_range_is_sorted_after_parallel_sorting", "[algorithm][sort][mt]" )
{
	std::vector<int> v{94, -1, 5, 3, 66, 3};

	mart::sort( std::execution::par, v );
	CHECK( mart::is_sorted( v ) );
}

TEST_CASE( "algo_sort_range_is_reverse_sorted_after_parallel_reverse_sorting", "[algorithm][sort][mt]" )
{
	std::vector<int> v{94, -1, 5, 3, 66, 3};

	mart::sort(std::execution::par, v, std::greater<>{} );
	CHECK( mart::is_sorted( v, std::greater<>{} ) );
}

#endif
