
#include <atomic>
#include <cassert>
#include <mutex>
#include <thread>

namespace mart {
namespace experimental {
namespace mt {

/**
 * This is a mutex optimized for the uncontested case
 * For all standard libraries except the msvc one, this is just forwarding to std::mutex
 *
 * The first thread to arrive just sets a flag
 * The second thread to arrive acquires a mutex and SPINS on the flag set by the first thread
 * All further threads just block on quiring the mutex
 *
 * The mutex is NOT FAIR
 *
 */

/**
 * Mutex with low overhead in uncontested case
 * but lets at least one waiting thread !!! SPIN !!!
 */
class FastMutexImpl {
	std::mutex       _mx;
	std::atomic_flag _flag{};            // ATOMIC_FLAG_INIT;
	bool             _mx_locked_by_me{}; // needs not be atomic as it is proteced by _flag and / or _mx
public:
	// Make mutex copy / movable
	FastMutexImpl() = default;
	FastMutexImpl( FastMutexImpl&& ){};
	FastMutexImpl( const FastMutexImpl& ){};
	FastMutexImpl& operator=( FastMutexImpl&& ) { return *this; };
	FastMutexImpl& operator=( const FastMutexImpl& ) { return *this; };

	void lock()
	{
		if( !try_lock() ) {
			_lock_slow();
			// Invariants at this point:
			// - _flag is set
			// - _mx is locked && _mx_locked_by_me == true
		}
		// Invariants at this point:
		// - _flag is set
		// - only one thread can ever be in the section between
		//   this point and the start of unlock() at the same time
		assert( _flag.test_and_set() );
	}

	inline bool try_lock()
	{
		bool res = _flag.test_and_set( std::memory_order_acquire ) == false;
		// ################## start of critical section if res == true ######################
		// Invariants at this point: _mx is not locked by this thread
		return res;
	}

	void unlock()
	{
		// Invariants at this point: same as at the end of lock()
		if( _mx_locked_by_me ) {
			_mx_locked_by_me = false;
			_mx.unlock();
		}
		// ~~~~~~~~~~~~~~~~~~ start of critical section ~~~~~~~~~~~~~~~~~~~~~~
		_flag.clear( std::memory_order_release );
	}

private:
	void _lock_slow()
	{
		_mx.lock();
		/**
		 * At this point in the code there are at most 2 threads in the critical section:
		 * - The first one got there by simply setting _flag to true
		 * - The second one couldn't acquire _flag, but the _mx
		 *
		 * This includes the scenario that:
		 * - Another thread (t2) alread set the _flag
		 * - this thread (t1) failed on try_lock()
		 * - before t1 locked _mx, t2 exited the critical section and cleared _flag
		 * - another thread (t3) came in, uscceeded at try_lock (we got victimized)
		 *
		 */
		while( _flag.test_and_set( std::memory_order_acquire ) ) {
			// spin
			// XXX: Investigate other possibilities
			std::this_thread::yield();
		}
		// ################## start of critical section ######################
		// we have to remember to unlock the mutex
		_mx_locked_by_me = true;
	}
};

#ifdef _MSC_VER
using FastMutex = FastMutexImpl;
#else
using FastMutex = std::mutex;
#endif

} // namespace mt
} // namespace experimental
} // namespace mart