#include <mart-common/experimental/circular_buffer.h>

#include <catch2/catch.hpp>

// check if buffer can be used as part of a constexpr variable
[[maybe_unused]] constexpr mart::circular_buffer<int, 10> cxpr_buffer;

TEST_CASE("circular_buffer_various", "[circular_buffer]")
{
	mart::circular_buffer<int,10> buffer;
	buffer.push_back( 1 );
	CHECK( buffer.size() == 1 );
	CHECK( buffer.pop_front() == 1 );
	CHECK( buffer.size() == 0 );

	buffer.push_back( 2 );
	buffer.push_back( 3 );
	buffer.push_back( 4 );
	buffer.push_back( 5 );
	buffer.push_back( 6 );
	buffer.push_back( 7 );

	CHECK( buffer.size() == 6 );
	CHECK( buffer.pop_front() == 2 );
	CHECK( buffer.pop_front() == 3 );
	CHECK( buffer.pop_front() == 4 );
	CHECK( buffer.pop_front() == 5 );
	CHECK( buffer.pop_front() == 6 );
	CHECK( buffer.pop_front() == 7 );
	CHECK( buffer.size() == 0 );

	buffer.push_back( 8 );
	buffer.push_back( 9 );
	buffer.push_back( 10 );
	buffer.push_back( 11 );
	buffer.push_back( 12 );
	buffer.push_back( 13 );

	CHECK( buffer.size() == 6 );
	CHECK( buffer[0] == 8 );
	CHECK( buffer[1] == 9 );
	CHECK( buffer[2] == 10 );
	CHECK( buffer[3] == 11 );
	CHECK( buffer[4] == 12 );
	CHECK( buffer[5] == 13 );

	CHECK( buffer.pop_front() == 8 );
	CHECK( buffer.pop_front() == 9 );
	CHECK( buffer.pop_front() == 10 );
	CHECK( buffer.pop_front() == 11 );
	CHECK( buffer.pop_front() == 12 );
	CHECK( buffer.pop_front() == 13 );
	CHECK( buffer.size() == 0 );
	CHECK( buffer.empty() );
}