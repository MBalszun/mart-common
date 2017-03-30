#ifndef LIB_MART_COMMON_GUARD_RANDOM_H
#define LIB_MART_COMMON_GUARD_RANDOM_H
/**
 * random.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: Convenience functions for getting random numbers
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <random>
#include <type_traits>

/* Proprietary Library Includes */
/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

/* ######## random ################################################ */
inline std::default_random_engine& getRandomEngine()
{
	thread_local std::default_random_engine rg( std::random_device{}() );
	return rg;
}

// Shorthand to get an random integral random number within a certain range
template <class T = int>
inline T getRandomInt( T min, T max )
{
	static_assert( std::is_integral<T>::value, "Parameters must be integral type" );
	return std::uniform_int_distribution<T>{min, max}( getRandomEngine() );
}

template <class T = int>
inline T getRandomInt( T max )
{
	static_assert( std::is_integral<T>::value, "Parameters must be integral type" );
	return std::uniform_int_distribution<T>{T( 0 ), max}( getRandomEngine() );
}

// Shorthand to get an random floating point number within a certain range
template <class T = double>
inline T getRandomFloat( T min, T max )
{
	static_assert( std::is_floating_point<T>::value, "Parameters must be floating point type" );
	return std::uniform_real_distribution<T>{min, max}( getRandomEngine() );
}

template <class T = double>
inline T getRandomFloat( T max = 1.0 )
{
	static_assert( std::is_floating_point<T>::value, "Parameters must be floating point type" );
	return std::uniform_real_distribution<T>{T( 0.0 ), max}( getRandomEngine() );
}

inline bool getRandomBool( double hitProb )
{
	return std::bernoulli_distribution{hitProb}( getRandomEngine() );
}
}

#endif