#include <im_str/im_str.hpp>

#include "include_catch.hpp"

#include <iostream>

using namespace ::mba;

TEST_CASE( "Split Position", "[im_str]" )
{
	im_str s( "Hello World" );
	{
		auto [h, w] = s.split_at( 5 );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_at( 5, im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_at( 5, im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_at( 5, im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
}

TEST_CASE( "Split Separator Single", "[im_str]" )
{
	im_str s( "Hello World" );
	{
		auto [h, w] = s.split_on_first( ' ' );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_on_first( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_on_first( ' ', im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_on_first( ' ', im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}

	{
		auto [h, w] = s.split_on_last( ' ' );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_on_last( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_on_last( ' ', im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_on_last( ' ', im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
}

TEST_CASE( "Split Separator multi", "[im_str]" )
{
	im_str s( "Hello My World" );
	{
		auto [h, w] = s.split_on_first();
		REQUIRE( h == "Hello" );
		REQUIRE( w == "My World" );
	}
	{
		auto [h, w] = s.split_on_last();
		REQUIRE( h == "Hello My" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_on_first( ' ' );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "My World" );
	}
	{
		auto [h, w] = s.split_on_first( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " My World" );
	}
	{
		auto [h, w] = s.split_on_first( ' ', im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "My World" );
	}
	{
		auto [h, w] = s.split_on_first( ' ', im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "My World" );
	}

	{
		auto [h, w] = s.split_on_last( ' ' );
		REQUIRE( h == "Hello My" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_on_last( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello My" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_on_last( ' ', im_str::Split::After );
		REQUIRE( h == "Hello My " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_on_last( ' ', im_str::Split::Drop );
		REQUIRE( h == "Hello My" );
		REQUIRE( w == "World" );
	}
}

TEST_CASE( "Split full", "[im_str]" )
{
	std::vector<im_str> ref {"Hello", "my", "dear!", "How", "are", "you?"};

	std::string base = "Hello my dear! How are you?";
	im_str      s( base );

	const auto words = s.split_full( ' ' );

	CHECK( words.size() == 6 );
	CHECK( std::equal( ref.begin(), ref.end(), words.begin(), words.end() ) );
}

TEST_CASE( "Split_on_nonexisting_char", "[im_str]" )
{
	im_str ref {"Hello"};

	{
		auto [first, second] = ref.split_on_first( 'r', im_str::Split::Before );
		CHECK( first == ref );
		CHECK( second == "" );
	}
	{
		auto [first, second] = ref.split_on_first( 'r', im_str::Split::After );
		CHECK( first == ref );
		CHECK( second == "" );
	}
	{
		auto [first, second] = ref.split_on_first( 'r', im_str::Split::Drop );
		CHECK( first == ref );
		CHECK( second == "" );
	}
}

TEST_CASE( "Split_empty_string", "[im_str]" )
{
	im_str ref {""};

	{
		auto [first, second] = ref.split_on_first( 'r', im_str::Split::Before );
		CHECK( first == ref );
		CHECK( second == "" );
	}
	{
		auto [first, second] = ref.split_on_first( 'r', im_str::Split::After );
		CHECK( first == ref );
		CHECK( second == "" );
	}
	{
		auto [first, second] = ref.split_on_first( 'r', im_str::Split::Drop );
		CHECK( first == ref );
		CHECK( second == "" );
	}
}

TEST_CASE( "Split_on_last_char", "[im_str]" )
{
	im_str ref {"Hello"};

	{
		auto [first, second] = ref.split_on_first( 'o', im_str::Split::Before );
		CHECK( first == "Hell" );
		CHECK( second == "o" );
	}
	{
		auto [first, second] = ref.split_on_first( 'o', im_str::Split::After );
		CHECK( first == "Hello" );
		CHECK( second == "" );
	}
	{
		auto [first, second] = ref.split_on_first( 'o', im_str::Split::Drop );
		CHECK( first == "Hell" );
		CHECK( second == "" );
	}

	{
		auto [first, second] = ref.split_on_last( 'o', im_str::Split::Before );
		CHECK( first == "Hell" );
		CHECK( second == "o" );
	}
	{
		auto [first, second] = ref.split_on_last( 'o', im_str::Split::After );
		CHECK( first == "Hello" );
		CHECK( second == "" );
	}
	{
		auto [first, second] = ref.split_on_last( 'o', im_str::Split::Drop );
		CHECK( first == "Hell" );
		CHECK( second == "" );
	}
}

TEST_CASE( "Split_on_first_char", "[im_str]" )
{
	im_str ref {"Hello"};

	{
		auto [first, second] = ref.split_on_first( 'H', im_str::Split::Before );
		CHECK( first == "" );
		CHECK( second == "Hello" );
	}
	{
		auto [first, second] = ref.split_on_first( 'H', im_str::Split::After );
		CHECK( first == "H" );
		CHECK( second == "ello" );
	}
	{
		auto [first, second] = ref.split_on_first( 'H', im_str::Split::Drop );
		CHECK( first == "" );
		CHECK( second == "ello" );
	}

	{
		auto [first, second] = ref.split_on_last( 'H', im_str::Split::Before );
		CHECK( first == "" );
		CHECK( second == "Hello" );
	}
	{
		auto [first, second] = ref.split_on_last( 'H', im_str::Split::After );
		CHECK( first == "H" );
		CHECK( second == "ello" );
	}
	{
		auto [first, second] = ref.split_on_last( 'H', im_str::Split::Drop );
		CHECK( first == "" );
		CHECK( second == "ello" );
	}
}



