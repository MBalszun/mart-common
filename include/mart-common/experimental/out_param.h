#pragma once

namespace mart {
namespace outp {

/*
 * template wrapper  to allow c# style out parameter where necessary:
 *
 * void read(out_parm<std::string> buffer) {
 *
 *	buffer = "Hello World!";
 *
 * }
 *
 * void bar() {
 * std::string tmp;
 *
 * read(out(tmp));
 *
 * }
 *
 * Use this when it is not bovius, that a parameter is an out parameter
 *
 */
template<class T>
class out_param {
	T& _data;

	explicit out_param(T& data) :_data(data) {}

	template<class U>
	friend out_param<U> out(U&);
public:
	operator T&() { return _data; }
			 T& get() { return _data; }
	out_param& operator=(const T& other) { _data = other;			return *this; }
	out_param& operator=(T&& other)		 { _data = std::move(other); return *this; }
};

template<class U>
out_param<U> out(const U& var)
{
	return out_param<U>(var);
}

}

using outp::out;
using outp::out_param;

}

