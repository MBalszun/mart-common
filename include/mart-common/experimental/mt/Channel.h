#pragma once

#include "UnblockException.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace mart {
namespace experimental {
namespace mt {

template <class T>
class Channel {
public:
	void send( const T& t )
	{
		std::lock_guard<std::mutex> _( _mx );
		_fifo.push( t );
		_cv_non_empty.notify_one();
	}

	void send( T&& t )
	{
		std::lock_guard<std::mutex> _( _mx );
		_fifo.push( std::move( t ) );
		_cv_non_empty.notify_one();
	}

	bool try_receive( T& _receive_target )
	{
		bool received = false;
		if( _mx.try_lock() ) {
			std::lock_guard<std::mutex> _( _mx, std::adopt_lock );
			if( !_fifo.empty() ) {
				_receive_target = std::move( _fifo.front() );
				_fifo.pop();
				received = true;
			}
		}
		return received;
	}

	void receive( T& receive_target )
	{
		std::unique_lock<std::mutex> ul( _mx );
		_cv_non_empty.wait( ul, [this] {
			return !_fifo.empty() || _cancel;
		} );
		if (_cancel) {
			_cancel = false;
			throw Canceled{};
		}
		receive_target = _fifo.front();
		_fifo.pop();
	}

	T receive()
	{
		T ret;
		receive(ret);
		return ret;
	}

	void cancel_read()
	{
		{
			std::lock_guard<std::mutex> _( _mx );
			_cancel = true;
		}
		_cv_non_empty.notify_all();
	}

	void clear()
	{
		std::lock_guard<std::mutex> _( _mx );
		decltype( _fifo ){}.swap( _fifo );
	}

	void operator<<( const T& v ) { send( v ); }
	void operator<<( T&& v ) { send( std::move( v ) ); }

	class receiving {
	public:
		receiving( Channel* ch, T& receive_target )
			: _ch( ch )
			, _receive_target( receive_target )
		{
		}
		receiving( receiving&& other )
			: _ch( other._ch )
			, _receive_target( other._receive_target )
		{
			other._ch = nullptr;
		}
		// checking for success -> non-blocking try_receive semantics
		operator bool()
		{
			auto from = _ch;
			_ch = nullptr;
			return from && from->try_receive( _receive_target );
		}
		// didn't check -> blocking receive semantics
		~receiving()
		{
			if( _ch ) _ch->receive( _receive_target );
		}

	private:
		Channel* _ch;
		T&		 _receive_target;
	};

	/**
	 * non-blocking try_receive *or* blocking receive function
	 * depending on whether the return value is converted to bool e.g. in an if statement:
	 *
	 * int v;
	 * channel<int> ch;
	 *
	 * ch >> v; //blocking -waits until value is available
	 *
	 * if (ch >> v) //non-blocking
	 */

	receiving operator>>( T& v ) { return receiving( this, v ); }

private:
	std::queue<T>			_fifo;
	std::mutex				_mx;
	std::condition_variable _cv_non_empty;
	bool					_cancel{};
};
}
}
}
