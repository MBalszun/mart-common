#include <im_str/im_str.hpp>

#include "include_catch.hpp"

#include <iostream>

using namespace ::mba;

TEST_CASE( "Split Position", "[im_str]" )
{
	im_str s( "Hello World" );
	{
		auto [h, w] = s.split( 5 );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split( 5, im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split( 5, im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split( 5, im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
}

TEST_CASE( "Split Separator Single", "[im_str]" )
{
	im_str s( "Hello World" );
	{
		auto [h, w] = s.split_first( ' ' );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_first( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_first( ' ', im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_first( ' ', im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}

	{
		auto [h, w] = s.split_last( ' ' );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_last( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_last( ' ', im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_last( ' ', im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "World" );
	}
}

TEST_CASE( "Split Separator multi", "[im_str]" )
{
	im_str s( "Hello My World" );
	{
		auto [h, w] = s.split_first();
		REQUIRE( h == "Hello" );
		REQUIRE( w == "My World" );
	}
	{
		auto [h, w] = s.split_last();
		REQUIRE( h == "Hello My" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_first( ' ' );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "My World" );
	}
	{
		auto [h, w] = s.split_first( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello" );
		REQUIRE( w == " My World" );
	}
	{
		auto [h, w] = s.split_first( ' ', im_str::Split::After );
		REQUIRE( h == "Hello " );
		REQUIRE( w == "My World" );
	}
	{
		auto [h, w] = s.split_first( ' ', im_str::Split::Drop );
		REQUIRE( h == "Hello" );
		REQUIRE( w == "My World" );
	}

	{
		auto [h, w] = s.split_last( ' ' );
		REQUIRE( h == "Hello My" );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_last( ' ', im_str::Split::Before );
		REQUIRE( h == "Hello My" );
		REQUIRE( w == " World" );
	}
	{
		auto [h, w] = s.split_last( ' ', im_str::Split::After );
		REQUIRE( h == "Hello My " );
		REQUIRE( w == "World" );
	}
	{
		auto [h, w] = s.split_last( ' ', im_str::Split::Drop );
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
