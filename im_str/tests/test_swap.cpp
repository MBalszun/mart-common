#include <im_str/im_str.hpp>

#include "include_catch.hpp"

#include <string>

using namespace std::string_literals;

TEST_CASE( "Swap im_str", "[im_str]" )
{
	mba::im_str str1{ "Hello"s };
	{
		mba::im_str str2{ "World"s };

		swap( str1, str2 );

		CHECK( str2 == "Hello" );
	}
	CHECK( str1 == "World" );
}

TEST_CASE( "Swap im_zstr", "[im_zstr]" )
{
	mba::im_zstr str1{ "Hello"s };
	{
		mba::im_zstr str2{ "World"s };

		swap( str1, str2 );

		CHECK( str2 == "Hello" );
	}
	CHECK( str1 == "World" );
}

// verify that accidental swaps with std::string_view or between im_str and im_zstr are prevented
#if __cpp_concepts >= 201810 // TODO: should be >= 201907 once msvc concept support is complete
namespace {
template<class T1, class T2>
concept can_be_swapped_adl = requires( T1 t1, T2 t2 )
{
	swap( t1, t2 );
};

namespace support {

using std::swap;
template<class T1, class T2>
concept can_be_swapped_adl_using = requires( T1 t1, T2 t2 )
{
	swap( t1, t2 );
};

} // namespace support

template<class T1, class T2>
concept can_be_swapped = can_be_swapped_adl<T1, T2> || support::can_be_swapped_adl_using<T1, T2>;

} // namespace

static_assert( can_be_swapped<std::string_view, std::string_view> );
static_assert( !can_be_swapped<std::string_view, int> );

static_assert( can_be_swapped<mba::im_str, mba::im_str> );
static_assert( can_be_swapped<mba::im_zstr, mba::im_zstr> );

static_assert( !can_be_swapped<mba::im_str, std::string_view> );
static_assert( !can_be_swapped<std::string_view, mba::im_str> );

static_assert( !can_be_swapped<mba::im_str, mba::im_zstr> );
static_assert( !can_be_swapped<mba::im_zstr, mba::im_str> );
static_assert( !can_be_swapped<mba::im_zstr, std::string_view> );
static_assert( !can_be_swapped<std::string_view, mba::im_zstr> );

#endif
