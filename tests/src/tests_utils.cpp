#include <mart-common/utils.h>

#include <catch2/catch.hpp>
#include <vector>

TEST_CASE("make_with_capacity_creates_vector_with_set_capacity", "[utils]")
{
	for (int i = 0; i < 20; ++i) {
		auto v = mart::make_with_capacity<std::vector<int>>(i);
		CHECK(v.capacity() == i);
	}
}

TEST_CASE("clamp_returns_value_when_in_interval", "[utils]")
{
	const int max = 5;
	const int min = 1;
	for (int i = min; i <= max; ++i) {
		const auto r = mart::clamp(i, min, max);
		CHECK(r == i);
	}

}

TEST_CASE("clamp_returns_min_when_below_interval", "[utils]")
{
	const int max = 5;
	const int min = 1;
	for (int i = min-10; i <= min; ++i) {
		const auto r = mart::clamp(i, min, max);
		CHECK(r == min);
	}

}

TEST_CASE("clamp_returns_max_when_above_interval", "[utils]")
{
	const int max = 5;
	const int min = 1;
	for (int i = max; i < max+10; ++i) {
		const auto r = mart::clamp(i, min, max);
		CHECK(r == max);
	}

}