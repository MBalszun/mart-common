#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_THREAD_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_THREAD_H
/**
 * Thread.h (mart-common/experimental/mt)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	raii wrapper for std::thread
 *
 */

#include <thread>

namespace mart {

class Thread {
public:
	enum class OnDestruction {
		Join,
		Detach
	};
	Thread() = default;

	template<class ... ARGS>
	Thread(ARGS&& ... args)
		: _thread(std::forward<ARGS>(args)...)
	{
	}

	Thread(Thread &&) = default;

	Thread& operator= (Thread&& other) {
		execute_exit_action();
		_thread = std::move(other._thread);
		_onExit = other._onExit;
	}

	void setExitAction(OnDestruction action) {
		_onExit = action;
	}

	void join() {
		_thread.join();
	}

	bool joinable() const {
		return _thread.joinable();
	}

	~Thread() {
		execute_exit_action();
	}

	std::thread& getThread() { return _thread; }
	const std::thread& getThread() const { return _thread; }
private:
	void execute_exit_action() {
		if (_thread.joinable()) {
			switch (_onExit) {
			case OnDestruction::Join:
				_thread.join();
				break;
			case OnDestruction::Detach:
				_thread.detach();
				break;
			}
		}
	}

	std::thread _thread;
	OnDestruction _onExit = OnDestruction::Join;
};

}

#endif