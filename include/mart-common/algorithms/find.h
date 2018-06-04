#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_FIND_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_FIND_H
/**
 * find.h (mart-common/algorithms)
 *
 * Copyright (R) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides wrappers around standard find algorithms
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

// IMPLEMENTATION NOTE:
// The primary range will always be taken by forwarding reference
// Note because we might want to handle rng and l value ranges differently,
// but because the return type will often be different for const and non-const
// ranges (const_iterator vs iterator)

// find
template<class R, class V>
auto find( R&& rng, const V& value ) -> decltype( std::begin( rng ) )
{
	return std::find( MART_COMMON_ALL( rng ), value );
}

// find_if
template<class R, class UnaryPredicate>
auto find_if( R&& rng, UnaryPredicate p ) -> decltype( std::begin( rng ) )
{
	return std::find_if( MART_COMMON_ALL( rng ), p );
}

// find_if_not
template<class R, class UnaryPredicate>
auto find_if_not( R&& rng, UnaryPredicate p ) -> decltype( std::begin( rng ) )
{
	return std::find_if_not( MART_COMMON_ALL( rng ), p );
}

// find_end
template<class R, class SR>
auto find_end( R&& rng, const SR& sr ) -> decltype( std::begin( rng ) )
{
	return std::find_end( MART_COMMON_ALL( rng ), MART_COMMON_ALL( sr ) );
}

template<class R, class SR, class BinaryPredicate>
auto find_end( R&& rng, const SR& sr, BinaryPredicate p ) -> decltype( std::begin( rng ) )
{
	return std::find_end( MART_COMMON_ALL( rng ), MART_COMMON_ALL( sr ), p );
}

// find_first_of
template<class R1, class R2>
auto find_first_of( R1&& rng1, const R2& rng2 ) -> decltype( std::begin( rng1 ) )
{
	return std::find_first_of( MART_COMMON_ALL( rng1 ), MART_COMMON_ALL( rng2 ) );
}

template<class R1, class R2, class BinaryPredicate_T>
auto find_first_of( R1&& rng1, const R2& rng2, BinaryPredicate_T bp ) -> decltype( std::begin( rng1 ) )
{
	return std::find_first_of( MART_COMMON_ALL( rng1 ), MART_COMMON_ALL( rng2 ), bp );
}

// adjacent_find
template<class R>
auto adjacent_find( R&& rng ) -> decltype( std::begin( rng ) )
{
	return std::adjacent_find( MART_COMMON_ALL( rng ) );
}

} // namespace mart

#undef MART_COMMON_ALL

#endif
