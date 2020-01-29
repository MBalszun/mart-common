#include <mart-common/experimental/ranges.h>

#include <catch2/catch.hpp>

TEST_CASE( "experimental_frange_compiles", "[experimental][ranges]" )
{
	auto r = mart::experimental::frange( 0.1, 0.3 ).step( 0.1 );
	(void)r;
}

TEST_CASE( "experimental_vrange_compiles", "[experimental][ranges]" )
{
	auto r = mart::experimental::vrange<float>( 0.1f, 0.3f ).step( 0.1f );
	(void)r;
}

TEST_CASE( "experimental_enum_default_range", "[experimental][ranges]" )
{
	enum test_vals { a, b, c };
	mart::experimental::DefaultEnumRange<test_vals> rnage();
}
