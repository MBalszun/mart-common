#pragma once
#include <type_traits>
#include <stdexcept>
#include <random>

namespace mart {

// narrow_cast(): a searchable way to do narrowing casts of values
template <class T, class U>
inline constexpr T narrow_cast(U u) noexcept
{
	static_assert(std::is_arithmetic<T>::value && std::is_arithmetic<U>::value, "Narrow cast can only be used for arithmetic types");
	return static_cast<T>(u);
}

struct narrowing_error : std::exception {};

namespace _impl_narrow {

template <class T, class U>
struct is_same_signedness
	: std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value> {};

template <class T, class U, bool SameSigndness= is_same_signedness<T,U>::value>
struct sign_check {
	static void check(T t,U u)
	{
		if ((t < T{}) != (u < U{})) {
			throw narrowing_error();
		}
	}
};

template <class T, class U>
struct sign_check<T,U,true> {
	static void check(T, U)
	{}
};

} // _impl_narrow


template <class T, class U>
inline T narrow(U u)
{
	T t = narrow_cast<T>(u);

	// this only does any work if T and U are of different signdness
	_impl_narrow::sign_check<T,U>::check(t, u);

	//check if roundtrip looses information
	if (static_cast<U>(t) != u) {
		throw narrowing_error();
	}

	return t;
}

template<class E>
using uType_t = typename std::underlying_type<E>::type;

template<class E>
constexpr uType_t<E> toUType(E e) { return static_cast<uType_t<E>>(e); }

template<class T = int>
T getRandomNumber(T min, T max)
{
	thread_local std::default_random_engine rg(std::random_device{}());
	return std::uniform_int_distribution<T>(min, max)(rg);
}

}//mart