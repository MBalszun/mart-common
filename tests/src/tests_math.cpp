#include <mart-common/math.h>

#include <catch2/catch.hpp>

#include <type_traits>

TEST_CASE( "square_ints", "[math]" )
{
	CHECK( mart::square( 0 ) == 0 );
	CHECK( mart::square( 234 ) == 234 * 234 );
	CHECK( mart::square( -234 ) == 234 * 234 );
}

TEST_CASE( "square_doubles", "[math]" )
{
	CHECK( mart::square( 0.0 ) == 0.0 );
	CHECK( mart::square( 234.3 ) == 234.3 * 234.3 );
	CHECK( mart::square( -234.3 ) == -234.3 * -234.3 );
}

namespace {

struct TestTypeSquared {
	double value;

	friend bool operator==( TestTypeSquared l, TestTypeSquared r ) { return l.value == r.value; }
};

struct TestTypeSingle {
	double value;

	friend TestTypeSquared operator*( TestTypeSingle l, TestTypeSingle r ) { return {l.value * r.value}; }
};

TEST_CASE( "square_user_defined", "[math]" )
{
	static_assert( std::is_same_v<decltype( mart::square( TestTypeSingle{} ) ), TestTypeSquared>,
				   "Return type of mart::square is wrong" );

	CHECK( mart::square( TestTypeSingle{0.0} ) == TestTypeSquared{0.0} );
	CHECK( mart::square( TestTypeSingle{234.3} ) == TestTypeSingle{234.3} * TestTypeSingle{234.3} );
	CHECK( mart::square( TestTypeSingle{-234.3} ) == TestTypeSingle{-234.3} * TestTypeSingle{-234.3} );
}

} // namespace
