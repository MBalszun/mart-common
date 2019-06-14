#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_NON_MOD_SEQUENCE_OPS_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_NON_MOD_SEQUENCE_OPS_H
/**
 * non-mod-sequence-ops.h (mart-common/algorithms)
 *
 * Copyright (R) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides wrappers around standard library non modifying sequence operations
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

template<class R, class Pred>
bool all_of( const R& c, Pred p )
{
	return std::all_of( MART_COMMON_ALL( c ), p );
}

template<class R, class Pred>
bool any_of( const R& c, Pred p )
{
	return std::any_of( MART_COMMON_ALL( c ), p );
}

template<class R, class Pred>
bool none_of( const R& c, Pred p )
{
	return std::none_of( MART_COMMON_ALL( c ), p );
}

template<class R, class T>
constexpr auto count(const R& r, const T& value)
{
	std::size_t cnt = 0;
	const auto& end = r.end();
	for (auto it = r.begin(); it != end; ++it) {
		cnt += *it == value;
	}
	return cnt;
	//return count( MART_COMMON_ALL( r ), value );
}

template<class R, class UnaryPredicate>
auto count_if( const R& r, UnaryPredicate p )
{
	return count_if( MART_COMMON_ALL( r ), p );
}

template< class R, class UnaryFunction >
UnaryFunction for_each(R&& r, UnaryFunction f)
{
   return std::for_each(MART_COMMON_ALL( r ),f);
}

template<class R1, class R2>
auto mismatch( R1&& r1, R2&& r2 )
{
	return std::mismatch(MART_COMMON_ALL( r1 ), MART_COMMON_ALL( r2 ));
}

template<class R1, class R2, class BinaryPredicate>
auto mismatch( R1&& r1, R2&& r2, BinaryPredicate p )
{
	return std::mismatch( MART_COMMON_ALL( r1 ), MART_COMMON_ALL( r2 ), p );
}

//#if __cplusplus >= 201703L

template<class R1, class R2>
auto search( R1&& r1, R2&& r2 )
{
	return std::search( MART_COMMON_ALL( r1 ), MART_COMMON_ALL( r2 ) );
}

template<class R1, class R2, class BinaryPredicate>
auto search( R1&& r1, R2&& r2, BinaryPredicate p )
{
	return std::search( MART_COMMON_ALL( r1 ), MART_COMMON_ALL( r2 ), p );
}

template<class R1, class Size, class T>
auto search_n( R1&& r1, Size size, const T& value )
{
	return std::search_n( MART_COMMON_ALL( r1 ), size, value );
}

template<class R1, class Size, class T, class BinaryPredicate>
auto search_n( R1&& r1, Size size, const T& value, BinaryPredicate p )
{
	return std::search_n( MART_COMMON_ALL( r1 ), size, value, p );
}

//#endif

} // namespace mart

#undef MART_COMMON_ALL

#endif
