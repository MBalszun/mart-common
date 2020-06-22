#include <mart-common/enum/EnumIdxArray.h>

#include <catch2/catch.hpp>
#include <iostream>

namespace {

enum class TestEnum : int {
	F22,
	B2,
	Mig29
};


constexpr std::size_t mart_enumCnt_impl(TestEnum*) {
	return 3;
}

constexpr mart::EnumIdxArray<int, TestEnum> arr1{ 1, 2, 3 };

static_assert( arr1[TestEnum::F22] == 1 );
static_assert( arr1[TestEnum::B2] == 2 );
static_assert( arr1[TestEnum::Mig29] == 3 );

constexpr mart::EnumIdxArray<std::string_view, TestEnum> arr2;
}

TEST_CASE("EnumIdxArray", "[mart-common]") {


	mart::EnumIdxArray<std::string_view, TestEnum> arr3{};

	arr3[TestEnum::F22] = "F22";
	arr3[TestEnum::B2]  = "B2";
	arr3[TestEnum::Mig29] = "Mig29";

	CHECK(arr3[TestEnum::F22]   == "F22");
	CHECK(arr3[TestEnum::B2]    == "B2");
	CHECK(arr3[TestEnum::Mig29] == "Mig29");

	for (std::string_view& e : arr3) {
		std::cout << e << std::endl;
	}
}




