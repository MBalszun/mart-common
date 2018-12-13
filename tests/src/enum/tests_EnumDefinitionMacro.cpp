#include <mart-common/enum/EnumDefinitionMacro.h>

#include <catch2/catch.hpp>
#include <type_traits>

MART_UTILS_DEFINE_ENUM( Airplains, int, 5, F114, A380, F22, Eurofighter, Cessna );

static_assert( std::is_same_v< std::underlying_type_t<Airplains>, int >, "" );

constexpr int foo( Airplains p )
{
	return static_cast<int>( p );
};

TEST_CASE( "enum_values", "[StringView]" )
{
	CHECK( foo(Airplains::F114) == 0 );
	CHECK( foo( Airplains::A380 ) == 1 );
	CHECK( foo( Airplains::F22 ) == 2 );
	CHECK( foo( Airplains::Eurofighter ) == 3 );
	CHECK( foo( Airplains::Cessna ) == 4 );

	int i = 0;
	for (auto e : mart::getEnums<Airplains>()) {
		CHECK( foo( e ) == i );
		i++;
	}

}

