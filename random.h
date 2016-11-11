#pragma once

#include <type_traits>
#include <random>

namespace mart {

/* ######## random ################################################ */
inline std::default_random_engine& getRandomEngine()
{
	thread_local std::default_random_engine rg(std::random_device{}());
	return rg;
}

// Shorthand to get an random integral random number within a certain range
template <class T = int>
inline T getRandomInt(T min, T max)
{
	static_assert(std::is_integral<T>::value, "Parameters must be integral type");
	return std::uniform_int_distribution<T>(min, max)(getRandomEngine());
}

// Shorthand to get an random floating point number within a certain range
template<class T = double>
inline T getRandomFloat(T min, T max)
{
	static_assert(std::is_floating_point<T>::value, "Parameters must be floating point type");
	return std::uniform_real_distribution<T>(min, max)(getRandomEngine());
}


inline bool getRandomBool(double hitProb)
{
	return std::bernoulli_distribution{ hitProb }(getRandomEngine());
}

}