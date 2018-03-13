#include <mart-common/MartVec.h>

#include <catch2/catch.hpp>
#include <type_traits>


namespace {
	struct Wrapper {
		int v;
		friend Wrapper operator*(Wrapper l, int r) { return { l.v*r }; }
		friend Wrapper operator*(int l, Wrapper r) { return { l*r.v }; }
		friend Wrapper operator*(Wrapper l, Wrapper r) { return { l.v*r.v }; }
		friend bool operator==(Wrapper l, Wrapper r) { return l.v==r.v; }
	};
}

TEST_CASE("Some_random_vector_math_code", "[vec]")
{

	const mart::Vec3D<Wrapper> expected{ 1, 2 };

	const mart::Vec3D<Wrapper> base{ 1,1 };
	const auto 	r = base * mart::Vec3D<int>{1, 2};
	static_assert(std::is_same_v<decltype(expected), decltype(r)>);
	CHECK(expected == r);
}