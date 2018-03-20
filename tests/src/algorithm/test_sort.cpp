#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <vector>

TEST_CASE( "algo_sort_range_is_sorted_after_sorting", "[algorithm][sort]" )
{
	std::vector<int> v{94, -1, 5, 3, 66, 3};

	mart::sort( v );
	CHECK( mart::is_sortded( v ) );
}
