#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <functional>
#include <vector>

#ifndef MART_COMMON_TEST_PARALLEL_ALGORITHMS
#define MART_COMMON_TEST_PARALLEL_ALGORITHMS 0
#endif

#ifdef _MSC_VER
#undef MART_COMMON_TEST_PARALLEL_ALGORITHMS
#define MART_COMMON_TEST_PARALLEL_ALGORITHMS 1
#endif

#if MART_COMMON_TEST_PARALLEL_ALGORITHMS
#include <execution>
#endif

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

TEST_CASE( "algo_sort_range_is_sorted_after_parallel_sorting", "[algorithm][sort][mt]" )
{
#if MART_COMMON_TEST_PARALLEL_ALGORITHMS

	std::vector<int> v{94, -1, 5, 3, 66, 3};

	mart::sort( std::execution::par, v );
	CHECK( mart::is_sorted( v ) );

#else
	CHECK( true );
#endif
}

TEST_CASE( "algo_sort_range_is_reverse_sorted_after_parallel_reverse_sorting", "[algorithm][sort][mt]" )
{
#if MART_COMMON_TEST_PARALLEL_ALGORITHMS

	std::vector<int> v{94, -1, 5, 3, 66, 3};
	mart::sort( std::execution::par, v, std::greater<>{} );
	CHECK( mart::is_sorted( v, std::greater<>{} ) );

#else
	CHECK( true );
#endif
}
