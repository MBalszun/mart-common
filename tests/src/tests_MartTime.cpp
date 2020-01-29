#include <mart-common/MartTime.h>

#include <catch2/catch.hpp>

// just make sure each function in MartTime.h gets used at least once
TEST_CASE( "MartTime_dummy", "[MartTime]" )
{
	using namespace mart::chrono_literals;

	CHECK( mart::nanoseconds {-1} == -1_ns );
	CHECK( mart::microseconds {1} == std::chrono::nanoseconds {1000} );
	CHECK( mart::milliseconds {-1} == mart::microseconds {-1000} );
	CHECK( mart::seconds {1} == std::chrono::milliseconds {1000} );
	CHECK( mart::minutes {-1} == mart::seconds {-60} );
	CHECK( mart::hours {1} == std::chrono::minutes {60} );
	CHECK( mart::days {-1} == mart::hours {-24} );
	CHECK( mart::years {1} == mart::days {365} );

	CHECK( 3_h == mart::abs( 3_h ) );
	CHECK( 10_s == mart::abs( -10_s ) );
	CHECK( 0_ms == mart::abs( -std::chrono::milliseconds {0} ) );

	[[maybe_unused]] auto now_time_point = mart::now();
	[[maybe_unused]] auto now_duration   = mart::timeSinceEpoch();

	[[maybe_unused]] auto now_duration_us = mart::us_SinceEpoch();
	[[maybe_unused]] auto now_duartion_ms = mart::ms_SinceEpoch();
	[[maybe_unused]] auto now_duartion_s  = mart::s_SinceEpoch();

	[[maybe_unused]] auto elapsed       = mart::passedTime( now_time_point );
	[[maybe_unused]] auto has_timed_out = mart::hasTimedOut( now_time_point, 5_ms );

	mart::Timer t1;
	mart::Timer t2( 5_ms );
	t1.reset();

	[[maybe_unused]] auto t2elapsed   = t2.elapsed();
	[[maybe_unused]] auto t2remaining = t2.remaining();

	for( mart::PeriodicScheduler sched( 500_us ); sched.loopCnt() < 3; sched.sleep() ) {

		CHECK( sched.invocationCnt() == ( std::size_t )( sched.loopCnt() + 1 ) );
		[[maybe_unused]] auto time1 = sched.getNextWakeTime();
		[[maybe_unused]] auto time2 = sched.remainingIntervalTime();
		[[maybe_unused]] auto time3 = sched.runtime();
	}
}
