#include <mart-common/experimental/mt/TrippleBuffer.h>

#include <catch2/catch.hpp>

#include <string>
#include <thread>

TEST_CASE( "TrippleBuffer_sync_can_handle_more_writes_than_reads", "[mt][TrippleBuffer]" )
{
	mart::experimental::mt::TrippleBuffer<std::string> buffer( "0" );

	for( int i = 0; i < 100; i += 2 ) {
		auto& wrt_buff1 = buffer.get_write_buffer();
		wrt_buff1       = std::to_string( i );
		buffer.commit();

		if( i % 3 == 0 ) {
			buffer.get_write_buffer() = std::to_string( i + 2 );
			buffer.commit();
		}

		auto& wrt_buff2 = buffer.get_write_buffer();
		wrt_buff2       = std::to_string( i + 1 );
		buffer.commit();

		buffer.fetch_update();
		auto& rd_buff = buffer.get_read_buffer();
		auto  r       = std::stoi( rd_buff );
		CHECK( r == i + 1 );
	}
}

TEST_CASE( "TrippleBuffer_sync_can_handle_more_reads_than_writes", "[mt][TrippleBuffer]" )
{
	mart::experimental::mt::TrippleBuffer<std::string> buffer( "0" );

	for( int i = 0; i < 100; ++i ) {
		auto& wrt_buff1 = buffer.get_write_buffer();
		wrt_buff1       = std::to_string( i );
		buffer.commit();

		CHECK( buffer.fetch_update() );
		auto& rd_buff1 = buffer.get_read_buffer();
		auto  r1       = std::stoi( rd_buff1 );
		CHECK( r1 == i );

		CHECK( !buffer.fetch_update() );
		auto& rd_buff2 = buffer.get_read_buffer();
		auto  r2       = std::stoi( rd_buff2 );
		CHECK( r2 == i );

		if( i % 3 == 0 ) {
			CHECK( !buffer.fetch_update() );
			auto r = std::stoi( buffer.get_read_buffer() );
			CHECK( r == i );
		}
	}
}

TEST_CASE( "TrippleBuffer_sync_can_handle_varying_write_read_ratios", "[mt][TrippleBuffer]" )
{
	mart::experimental::mt::TrippleBuffer<std::string> buffer( "0" );

	for( int i = 0; i < 100; ++i ) {

		if( i % 4 == 0 ) {
			buffer.get_write_buffer() = std::to_string( i + 1 );
			buffer.commit();
		}

		if( i % 2 == 0 ) {
			buffer.get_write_buffer() = std::to_string( i + 2 );
			buffer.commit();
		}

		auto& wrt_buff = buffer.get_write_buffer();
		wrt_buff       = std::to_string( i );
		buffer.commit();

		CHECK( buffer.fetch_update() );
		auto& rd_buff = buffer.get_read_buffer();
		auto  r1      = std::stoi( rd_buff );
		CHECK( r1 == i );

		if( i % 2 == 1 ) {
			REQUIRE( !buffer.fetch_update() );
			auto r = std::stoi( buffer.get_read_buffer() );
			CHECK( r == i );
		}

		if( i % 4 == 1 ) {
			CHECK( !buffer.fetch_update() );
			auto r = std::stoi( buffer.get_read_buffer() );
			CHECK( r == i );
		}
	}
}

TEST_CASE( "TrippleBuffer_mt_complex_producer_consumer", "[mt][TrippleBuffer][threaded_test]" )
{
	static constexpr int ItCnt = 5000;

	mart::experimental::mt::TrippleBuffer<std::string> buffer( "-1" );

	auto producer = [&buffer]() {
		for( int i = 0; i < ItCnt; ++i ) {
			std::string& t = buffer.get_write_buffer();

			t = std::to_string( i );

			buffer.commit();
		}
	};
	auto consumer = [&buffer]() {
		int last = -1;
		for( ; last < ItCnt - 1; ) {
			bool newData = buffer.fetch_update();

			std::string& t = buffer.get_read_buffer();
			auto         r = std::stoi( t );

			CHECK( r < ItCnt );
			CHECK( last <= r );

			if( newData ) { CHECK( last < r ); }

			last = r;
		}
	};

	std::thread ct( consumer );
	std::thread pt( producer );

	pt.join();
	ct.join();
}

TEST_CASE( "TrippleBuffer_mt_simple_producer_consumer", "[mt][TrippleBuffer][threaded_test]" )
{
	static constexpr int ItCnt = 500'000;

	mart::experimental::mt::TrippleBuffer<int> buffer( -1 );

	auto producer = [&buffer]() {
		for( int i = 0; i < ItCnt; ++i ) {
			buffer.get_write_buffer() = i;
			buffer.commit();
		}
	};
	auto consumer = [&buffer]() {
		int last = -1;
		for( ; last < ItCnt - 1; ) {
			bool newData = buffer.fetch_update();

			auto r = buffer.get_read_buffer();
			CHECK( r < ItCnt );
			CHECK( last <= r );

			if( newData ) { CHECK( last < r ); }

			last = r;
		}
	};

	std::thread ct( consumer );
	std::thread pt( producer );

	pt.join();
	ct.join();
}

namespace {
struct LargePod {
	explicit LargePod( int i ) { fill( i ); }
	void fill( int i ) { std::fill( data.begin(), data.end(), i ); }
	bool is_consistent() const
	{
		auto abnormals = std::count_if( data.begin(), data.end(), [this]( auto i ) { return i != data[0]; } );
		return abnormals == 0;
	}
	std::array<int, 211> data; // spanning multiple cache lines
};
} // namespace

TEST_CASE( "TrippleBuffer_mt_large_pod_producer_consumer", "[mt][TrippleBuffer][threaded_test]" )
{
	static constexpr int ItCnt = 500'000;

	mart::experimental::mt::TrippleBuffer<LargePod> buffer( LargePod {-1} );

	auto producer = [&buffer]() {
		for( int i = 0; i < ItCnt; ++i ) {
			buffer.get_write_buffer().fill( i );
			buffer.commit();
		}
	};
	auto consumer = [&buffer]() {
		int last = -1;
		for( ; last < ItCnt - 1; ) {
			bool newData = buffer.fetch_update();

			auto& b = buffer.get_read_buffer();
			CHECK( b.is_consistent() );

			auto r = b.data[0];
			CHECK( r < ItCnt );
			CHECK( last <= r );

			if( newData ) { CHECK( last < r ); }

			last = r;
		}
	};

	std::thread ct( consumer );
	std::thread pt( producer );

	pt.join();
	ct.join();
}
