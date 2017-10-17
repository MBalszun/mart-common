
#include <mutex>
#include <atomic>

namespace mart {
namespace experimental {
namespace mt {

/**
* This is a mutex optimized for the uncontested case
*
* The first thread to arrive just sets a flag
* The second thread to arrive acquires a mutex and SPINS on the flag set by the first thread
* All further threads just block on quiring the mutex
*
* The mutex is NOT FAIR
*
*/

class FastMutex {
	std::mutex _mx;
	std::atomic_flag _flag{};// ATOMIC_FLAG_INIT;
	bool _mx_locked_by_me{}; //needs not be atomic as it is proteced by _flag and / or _mx
public:
	// Make mutex copy / movable
	FastMutex() = default;
	FastMutex(FastMutex&& ) {};
	FastMutex(const FastMutex& ) {};
	FastMutex& operator=(FastMutex&& ) { return *this; };
	FastMutex& operator=(const FastMutex& ) { return *this; };

	void lock() {
		if (try_lock()) {
			// fast path
			return;
		} else {
			_lock_slow();
		}
	}

	bool try_lock() {
		return _flag.test_and_set(std::memory_order_acquire) == false;
	}

	void unlock() {
		// At this point we are the only thread that could possibly write or
		const auto t = _mx_locked_by_me;
		_flag.clear(std::memory_order_release);
		//moving the slow unlock procedure after _flag clear is a small optimization
		if (t) {
			_mx_locked_by_me = false;
			_mx.unlock();
		}

	}
private:
	void _lock_slow() {
		_mx.lock();
		/**
		 * At this point in the code there are at most 2 threads in the critical section:
		 * - The first one got there by simply setting _flag to true
		 * - The second one couldn't acquire the flag, but the mutex
		 *
		 * Any further thread that calls lock now will block on the mutex
		 *
		 * Another possible scenario is:
		 * - We are the only thread that
		 *

		 *
		 */
		while (_flag.test_and_set(std::memory_order_acquire)) {
			// spin
			//XXX: Investigate other possibilities
			std::this_thread::yield();
		}
		//we have to remember to unlock the mutex
		_mx_locked_by_me = true;
	}
};

}
}
}