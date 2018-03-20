#include <mart-common/TypePunningUnion.h>

#include <catch2/catch.hpp>

#include <mart-common/ArrayView.h>

namespace {
struct S1 {
	float f1;
	float f2;
	float f3;
};

struct S2 {
	int f1;
	int f2;
	int f3;
};

struct S3 {
	float f[3];
};

struct S4 {
	float f;
};

}

TEST_CASE( "TypePunningUnion_dummy", "[TypePunningUnion]" )
{
	mart::TypePunningUnion<S1, S2, S3, S4> my_union1;
	mart::TypePunningUnion<S1, S2, S3, S4> my_union2;

	auto my_union3 = my_union1;
	my_union3 = my_union2;

	my_union3 = S3{ 1.0,1.0,2.0 };
	const S1 s1{2.0,2.0,3.0};
	auto mem_view_s1 = mart::viewMemoryConst(s1);

	my_union3 = mem_view_s1;

	my_union3 = S4{ 1.0 };
	CHECK(my_union3.size() == sizeof(S4));

	auto ab3 = my_union3.all_bytes();
	CHECK(ab3.size() >= sizeof(S1));

	auto vb3 = my_union3.valid_bytes();
	CHECK(vb3.size() == sizeof(S4));

	S4 data = my_union3.as<S4>();

	mart::copy(mem_view_s1, ab3);
	my_union3.set_active_range(ab3.subview(0, mem_view_s1.size()));
}
