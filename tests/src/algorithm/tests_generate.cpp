#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <array>

TEST_CASE( "generate_fills_array", "[algorithms][find]" )
{
	auto gen1 = [i = 0] () mutable { return i++; };
	auto gen2 = gen1;

	std::array<int, 6> expected{gen1(), gen1(), gen1(), gen1(), gen1(), gen1()};
	std::array<int, 6> data{};
	mart::generate( data, gen2 );
	CHECK( data == expected );
}
