#include <mart-common/utils.h>

#include <catch2/catch.hpp>
#include <vector>

TEST_CASE( "make_with_capacity_creates_vector_with_set_capacity", "[utils]" )
{
	for( int i = 0; i < 20; ++i ) {
		auto v = mart::make_with_capacity<std::vector<int>>( i );
		CHECK( v.capacity() == i );
	}
}

TEST_CASE( "clamp_returns_value_when_in_interval", "[utils]" )
{
	const int max = 5;
	const int min = 1;
	for( int i = min; i <= max; ++i ) {
		const auto r = mart::clamp( i, min, max );
		CHECK( r == i );
	}
}

TEST_CASE( "clamp_returns_min_when_below_interval", "[utils]" )
{
	const int max = 5;
	const int min = 1;
	for( int i = min - 10; i <= min; ++i ) {
		const auto r = mart::clamp( i, min, max );
		CHECK( r == min );
	}
}

TEST_CASE( "clamp_returns_max_when_above_interval", "[utils]" )
{
	const int max = 5;
	const int min = 1;
	for( int i = max; i < max + 10; ++i ) {
		const auto r = mart::clamp( i, min, max );
		CHECK( r == max );
	}
}

TEST_CASE( "narrow_is_silent_when_no_loss", "[utils]" )
{
	bool no_exception = true;
	try {
		auto i = mart::narrow<unsigned int>( 1000 );
		CHECK( i == 1000u );
	} catch( ... ) {
		no_exception = false;
	}
	CHECK( no_exception );
}

TEST_CASE( "narrow_throws_when_negative_int_is_cast_to_unsigned", "[utils]" )
{
	bool no_exception = true;
	try {
		[[maybe_unused]] auto i = mart::narrow<unsigned int>( -1000 );
	} catch( ... ) {
		no_exception = false;
	}
	CHECK( no_exception == false );
}

TEST_CASE( "narrow_throws_when_target_type_is_too_small", "[utils]" )
{
	bool no_exception = true;
	try {
		[[maybe_unused]] auto i = mart::narrow<int>( 13044212123412ll );
	} catch( ... ) {
		no_exception = false;
	}
	CHECK( no_exception == false );
}

TEST_CASE( "narrow_cast", "[utils]" )
{
	[[maybe_unused]] auto i = mart::narrow_cast<int>( 5ll );
}

TEST_CASE( "type_is_one_of", "[utils]" )
{
	CHECK( mart::type_is_any_of<int, long, char, int, double>() );
	CHECK( !mart::type_is_any_of<int, long, char, double>() );
}

static_assert( mart::type_is_any_of<int, long, char, int, double>() );
static_assert( !mart::type_is_any_of<int, long, char, double>() );

TEST_CASE( "execute_on_exit", "[utils]" )
{
	bool flag_was_set = false;
	{
		mart::ExecuteOnExit_t guard( [&] { flag_was_set = true; } );
		CHECK( flag_was_set == false );
	}
	CHECK( flag_was_set );
}

TEST_CASE( "move_if", "[utils]" )
{
	// NOTE: currently all tests are performed at compiletime, as we ar just checking types
	int         i1{};
	int&        i2 = i1;
	const int   i3{};
	const int&  i4 = i3;
	int&&       i5 = std::move( i1 );
	const int&& i6 = std::move( i3 );

	static_assert( std::is_same_v<decltype( mart::move_if<true>( i1 ) ), decltype( std::move( i1 ) )> );
	static_assert( std::is_same_v<decltype( mart::move_if<true>( i2 ) ), decltype( std::move( i2 ) )> );
	static_assert( std::is_same_v<decltype( mart::move_if<true>( i3 ) ), decltype( std::move( i3 ) )> );
	static_assert( std::is_same_v<decltype( mart::move_if<true>( i4 ) ), decltype( std::move( i4 ) )> );
	static_assert( std::is_same_v<decltype( mart::move_if<true>( i5 ) ), decltype( std::move( i5 ) )> );
	static_assert( std::is_same_v<decltype( mart::move_if<true>( i6 ) ), decltype( std::move( i6 ) )> );

	static_assert( std::is_same_v<decltype( mart::move_if<false>( i1 ) ), int&> );
	static_assert( std::is_same_v<decltype( mart::move_if<false>( i2 ) ), int&> );
	static_assert( std::is_same_v<decltype( mart::move_if<false>( i3 ) ), const int&> );
	static_assert( std::is_same_v<decltype( mart::move_if<false>( i4 ) ), const int&> );
	static_assert( std::is_same_v<decltype( mart::move_if<false>( i5 ) ), int&> );
	static_assert( std::is_same_v<decltype( mart::move_if<false>( i6 ) ), const int&> );
}
