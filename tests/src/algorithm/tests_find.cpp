#include <mart-common/algorithm.h>

#include <catch2/catch.hpp>

#include <array>


TEST_CASE("find_returns_iterator_to_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find(data, 2);
	CHECK(it == data.begin() + 1);
}

TEST_CASE("find_returns_end_iterator_if_element_doesnt_exist", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find(data, 0);
	CHECK(it == data.end());
}

TEST_CASE("find_returns_end_iterator_on_empty_range", "[algorithms][find]")
{
	std::array<int, 0> data{};
	auto it = mart::find(data, 0);
	CHECK(it == data.end());
}

TEST_CASE("find_finds_first_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find(data, 1);
	CHECK(it == data.begin());
}

TEST_CASE("find_finds_last_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find(data, 5);
	CHECK(it == data.end()-1);
}


/* find_if*/
TEST_CASE("find_if_returns_iterator_to_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if(data, [](auto e) {return e == 2; });
	CHECK(it == data.begin() + 1);
}

TEST_CASE("find_if_returns_end_iterator_if_element_doesnt_exist", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if(data, [](auto e) {return e == 0; });
	CHECK(it == data.end());
}

TEST_CASE("find_if_returns_end_iterator_on_empty_range", "[algorithms][find]")
{
	std::array<int, 0> data{};
	auto it = mart::find_if(data, [](auto e) {return e == 0; });
	CHECK(it == data.end());
}

TEST_CASE("find_if_finds_first_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if(data, [](auto e) {return e == 1; });
	CHECK(it == data.begin());
}

TEST_CASE("find_if_finds_last_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if(data, [](auto e) {return e == 5; });
	CHECK(it == data.end() - 1);
}


/* find_if_ex*/
TEST_CASE("find_if_ex_returns_iterator_to_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if_ex(data, [](auto e) {return e == 2; });
	CHECK(it);
	CHECK(it == data.begin() + 1);
}

TEST_CASE("find_if_ex_returns_end_iterator_if_element_doesnt_exist", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if_ex(data, [](auto e) {return e == 0; });
	CHECK(!it);
	CHECK(it == data.end());
}

TEST_CASE("find_if_ex_returns_end_iterator_on_empty_range", "[algorithms][find]")
{
	std::array<int, 0> data{};
	auto it = mart::find_if_ex(data, [](auto e) {return e == 0; });
	CHECK(!it);
	CHECK(it == data.end());
}

TEST_CASE("find_if_ex_finds_first_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if_ex(data, [](auto e) {return e == 1; });
	CHECK(it);
	CHECK(it == data.begin());
}

TEST_CASE("find_if_ex_finds_last_element", "[algorithms][find]")
{
	std::array<int, 5> data{ 1,2,3,4,5 };
	auto it = mart::find_if_ex(data, [](auto e) {return e == 5; });
	CHECK(it);
	CHECK(it == data.end() - 1);
}
