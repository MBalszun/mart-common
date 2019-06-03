#include <im_str/im_str.hpp>

#include "include_catch.hpp"

#include <iostream>
#include <string>
#include <thread>

using namespace std::literals;

void requireZero( std::string_view view )
{
	REQUIRE( view.data()[view.size()] == '\0' );
}

template<class Container>
constexpr bool has_standard_container_typedefs()
{
	using T = Container;
	[[maybe_unused]] typename T::traits_type            traits {};
	[[maybe_unused]] typename T::value_type             v {};
	[[maybe_unused]] typename T::pointer                p {};
	[[maybe_unused]] typename T::const_pointer          cp {};
	[[maybe_unused]] typename T::reference              r  = v;
	[[maybe_unused]] typename T::const_reference        cr = v;
	[[maybe_unused]] typename T::const_iterator         cit {};
	[[maybe_unused]] typename T::iterator               it {};
	[[maybe_unused]] typename T::reverse_iterator       rit {};
	[[maybe_unused]] typename T::const_reverse_iterator crit {};
	[[maybe_unused]] typename T::size_type              s {};
	[[maybe_unused]] typename T::difference_type        d {};
	return true;
}

void static_checks()
{
	static_assert( has_standard_container_typedefs<mba::im_str>() );
	static_assert( has_standard_container_typedefs<mba::im_zstr>() );
}

TEST_CASE( "Construction from literal", "[im_str]" )
{
	{
		mba::im_str str1 = "Hello World";
		REQUIRE( str1 == "Hello World" );

		mba::im_str str2 {"Hello World"};
		REQUIRE( str2 == "Hello World" );

		mba::im_str str3 = {"Hello World"};
		REQUIRE( str3 == "Hello World" );
	}

	{
		mba::im_zstr zstr1 = "Hello World";
		REQUIRE( zstr1 == "Hello World" );

		mba::im_zstr zstr2 {"Hello World"};
		REQUIRE( zstr2 == "Hello World" );

		mba::im_zstr zstr3 = {"Hello World"};
		REQUIRE( zstr3 == "Hello World" );
	}
}

TEST_CASE( "Construction empty", "[im_str]" )
{
	{
		mba::im_str str1 = "";
		mba::im_str str2 {};
		REQUIRE( str1 == str2 );
	}
	{
		mba::im_zstr zstr1 = "";
		mba::im_zstr zstr2 {};
		REQUIRE( zstr1 == zstr2 );
	}
}

TEST_CASE( "Construction from std::string", "[im_str]" )
{
	{
		mba::im_str str1( "Hello World" );
		REQUIRE( str1 == "Hello World" );

		mba::im_str str2 {"Hello World"s};
		REQUIRE( str2 == "Hello World" );

		auto stdstr = "Hello World"s;

		mba::im_str str3 {stdstr};
		REQUIRE( str3 == "Hello World" );
	}
	{
		mba::im_zstr zstr1( "Hello World" );
		REQUIRE( zstr1 == "Hello World" );

		mba::im_zstr zstr2 {"Hello World"s};
		REQUIRE( zstr2 == "Hello World" );

		auto stdstr = "Hello World"s;

		mba::im_str zstr3 {stdstr};
		REQUIRE( zstr3 == "Hello World" );
	}
}

TEST_CASE( "Construction from temporary std::string", "[im_str]" )
{
	{
		mba::im_str cs = [] {
			auto        stdstr = "Hello World"s;
			mba::im_str cs {stdstr};
			stdstr[0] = 'M'; // modify original string to make sure we really have an independent copy
			REQUIRE( cs != stdstr );
			return cs;
		}();
		REQUIRE( cs == "Hello World" );
	}
	{
		mba::im_zstr cs = [] {
			auto         stdstr = "Hello World"s;
			mba::im_zstr cs {stdstr};
			stdstr[0] = 'M'; // modify original string to make sure we really have an independent copy
			REQUIRE( cs != stdstr );
			return cs;
		}();
		REQUIRE( cs == "Hello World" );
	}
}

TEST_CASE( "Copy", "[im_str]" )
{
	{
		mba::im_str str1;
		{
			// no heap allocation
			mba::im_str tcs = "Hello World";
			str1            = tcs;
		}
		REQUIRE( str1 == "Hello World" );

		mba::im_str str2;
		{
			// heap allocated
			mba::im_str tcs {"Hello World"s};
			str2 = tcs;
		}
		REQUIRE( str2 == "Hello World" );
	}
	{
		mba::im_zstr str1;
		{
			// no heap allocation
			mba::im_zstr tcs = "Hello World";
			str1             = tcs;
		}
		REQUIRE( str1 == "Hello World" );

		mba::im_zstr str2;
		{
			// heap allocated
			mba::im_zstr tcs {"Hello World"s};
			str2 = tcs;
		}
		REQUIRE( str2 == "Hello World" );
	}
}

TEST_CASE( "concat", "[im_str]" )
{
	mba::im_str cs       = "How are you?";
	auto        combined = concat( "Hello", " World! "s, cs );
	REQUIRE( combined == "Hello World! How are you?" );
	REQUIRE( combined.is_zero_terminated() );
	requireZero( combined );
}

TEST_CASE( "comparison", "[im_str]" )
{
	mba::im_str str1 = "Hello1";
	mba::im_str str2 = "Hello2";

	CHECK( str1 < str2 );
	CHECK( str1 > "Hello" );
	CHECK( str2 < std::string( "Hello2o" ) );
}

TEST_CASE( "thread" )
{
	constexpr int iterations = 100'000;
	using namespace std::literals;
	const std::string    cpps1 = "Good";
	const std::string    cpps2 = "Bad";
	mba::im_str          s1 {"Hello"s};
	mba::im_str          s2 {"World!"s};
	std::atomic_uint64_t total = 0;

	std::atomic_int total_cpps1_fail_cnt = 0;
	std::atomic_int total_cpps2_fail_cnt = 0;
	std::atomic_int total_s1_fail_cnt    = 0;
	std::atomic_int total_s2_fail_cnt    = 0;

	auto f = [&, s1, s2] {
		std::size_t sum            = 0;
		int         cpps1_fail_cnt = 0;
		int         cpps2_fail_cnt = 0;
		int         s1_fail_cnt    = 0;
		int         s2_fail_cnt    = 0;
		for( int i = 0; i < iterations; i++ ) {
			{
				mba::im_str cs {cpps1};
				sum += cs.size();
				cpps1_fail_cnt += cs[0] != 'G';
			}
			{
				mba::im_str cs {cpps2};
				sum += cs.size();
				cpps2_fail_cnt += cs[0] != 'B';
			}
			{
				auto s = s1;
				sum += s.size();
				s1_fail_cnt += s[0] != 'H';
				s = s2;
				s2_fail_cnt += s[0] != 'W';
				sum += s.size();
			}
		}
		total += sum;
		total_cpps1_fail_cnt += cpps1_fail_cnt;
		total_cpps2_fail_cnt += cpps2_fail_cnt;
		total_s1_fail_cnt += s1_fail_cnt;
		total_s2_fail_cnt += s2_fail_cnt;
	};
	std::thread th1( f );
	std::thread th2( f );
	th1.join();
	th2.join();
	REQUIRE( total == ( s1.size() + s2.size() + cpps1.size() + cpps2.size() ) * 2 * iterations );
	REQUIRE( total_cpps1_fail_cnt == 0 );
	REQUIRE( total_cpps2_fail_cnt == 0 );
	REQUIRE( total_s1_fail_cnt == 0 );
	REQUIRE( total_s2_fail_cnt == 0 );
}
