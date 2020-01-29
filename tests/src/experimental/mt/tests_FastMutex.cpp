#include <mart-common/experimental/mt/FastMutex.h>

#include <catch2/catch.hpp>

#include <mutex>
#include <thread>

namespace {

template<class MX>
void lock_unlock( MX& mx, int times )
{
	for( int i = 0; i < times; ++i ) {
		std::lock_guard<MX> lg( mx );
	}
}

template<class MX, class F, class Init_t>
auto lock_unlock( MX& mx, int times, F func, Init_t init ) -> Init_t
{
	for( int i = 0; i < times; ++i ) {
		std::lock_guard<MX> lg( mx );
		init = func( init, i );
	}
	return init;
}

} // namespace

TEST_CASE( "benchmark_FastMutexImpl_lock_and_unlock_std_reference_st", "[mt][FastMutexImpl][!benchmark]" )
{
#ifdef MART_COMMON_RUN_BENCHMARK
	std::mutex mx;
	BENCHMARK( "lock_unlock_std_mutex" ) { lock_unlock( mx, 10'000'000 ); };
#endif
}

TEST_CASE( "benchmark_FastMutexImpl_lock_and_unlock_st", "[mt][FastMutexImpl][!benchmark]" )
{
#ifdef MART_COMMON_RUN_BENCHMARK
	mart::experimental::mt::FastMutexImpl mx;
	BENCHMARK( "lock_unlock_fast_mutex" ) { lock_unlock( mx, 10'000'000 ); };
#endif
}

TEST_CASE( "benchmark_FastMutexImpl_multi_threaded_lock_and_unlock_std_reference", "[mt][FastMutexImpl][!benchmark]" )
{
#ifdef MART_COMMON_RUN_BENCHMARK
	std::mutex mx;
	BENCHMARK( "lock_unlock_std_mutex_mt" )
	{
		std::thread th1{[&] { lock_unlock( mx, 10'000'000 ); }};
		std::thread th2{[&] { lock_unlock( mx, 10'000'000 ); }};
		th1.join();
		th2.join();
	};
#endif
}

TEST_CASE( "benchmark_FastMutexImpl_single_threaded_lock_and_unlock", "[mt][FastMutexImpl][!benchmark]" )
{
#ifdef MART_COMMON_RUN_BENCHMARK
	mart::experimental::mt::FastMutexImpl mx;
	BENCHMARK( "lock_unlock_fast_mutex_mt" )
	{
		std::thread th1{[&] { lock_unlock( mx, 10'000'000 ); }};
		std::thread th2{[&] { lock_unlock( mx, 10'000'000 ); }};
		th1.join();
		th2.join();
	};
#endif
}

TEST_CASE( "FastMutex_signle_threaded_lock_and_unlock", "[mt][FastMutex]" )
{
	mart::experimental::mt::FastMutex mx;
	lock_unlock( mx, 1'000'000 );
}
