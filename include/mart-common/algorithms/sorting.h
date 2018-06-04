#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_SORTING_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_SORTING_H
/**
 * sortings.h (mart-common/algorithms)
 *
 * Copyright (R) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides wrappers around standard library modifying sequence operations
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
/* Proprietary Library Includes */
/* Standard Library Includes */
#include <algorithm>
#include <iterator>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

// Will be undefined at end of file
#define MART_COMMON_ALL( rng ) std::begin( rng ), std::end( rng )

namespace mart {

template<class R>
void sort( R& rng )
{
	std::sort( MART_COMMON_ALL( rng ) );
}

// TODO: sort( <range>, <comparator> ) is still implemented in algorithm.h as disambiguation with parallel form requries
// more trickery

template<class R>
void stable_sort( R& rng )
{
	std::stable_sort( MART_COMMON_ALL( rng ) );
}

template<class R, class Compare>
void stable_sort( R& rng, Compare cmp )
{
	std::stable_sort( MART_COMMON_ALL( rng ), cmp );
}

template<class R, class RandomIt>
void partial_sort( R& rng, RandomIt middle )
{
	std::partial_sort( rng.begin(), middle, rng.end() );
}

template<class R, class RandomIt, class Compare>
void partial_sort( R& rng, RandomIt middle, Compare cmp )
{
	std::partial_sort( rng.begin(), middle, rng.end(), cmp );
}

template<class R, class RandomIt>
void nth_element( R& rng, RandomIt nth )
{
	std::nth_element( rng.begin(), nth, rng.end() );
}

template<class R, class RandomIt, class Compare>
void nth_element( R& rng, RandomIt nth, Compare cmp )
{
	std::nth_element( rng.begin(), nth, rng.end(), cmp );
}

template<class R>
bool is_sorted( const R& rng )
{
	return std::is_sorted( MART_COMMON_ALL( rng ) );
}

template<class R, class Comp>
bool is_sorted( const R& rng, Comp comp )
{
	return std::is_sorted( MART_COMMON_ALL( rng ), comp );
}

template<class R>
auto is_sorted_until( const R& rng )
{
	return std::is_sorted_until( MART_COMMON_ALL( rng ) );
}

template<class R, class Comp>
auto is_sorted_until( const R& rng, Comp comp )
{
	return std::is_sorted_until( MART_COMMON_ALL( rng ), comp );
}

} // namespace mart

#undef MART_COMMON_ALL

#endif
