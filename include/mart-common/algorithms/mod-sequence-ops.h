#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_MOD_SEQUENCE_OPS_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_MOD_SEQUENCE_OPS_H
/**
 * mod-sequence-ops.h (mart-common/algorithms)
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
#include <cassert>
#include <iterator>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

// Will be undefined at end of file
#define MART_COMMON_ALL( rng ) std::begin( rng ), std::end( rng )

namespace mart {

template<class SrcRng, class DestIt>
auto copy( const SrcRng& src, DestIt dest_it ) -> decltype( std::copy( MART_COMMON_ALL( src ), dest_it ) )
{
	return std::copy( MART_COMMON_ALL( src ), dest_it );
}

template<class SrcRng, class DestIt, class Pred>
auto copy_if( const SrcRng& src, DestIt dest_it, Pred p )
	-> decltype( std::copy_if( MART_COMMON_ALL( src ), dest_it, p ) )
{
	return std::copy_if( MART_COMMON_ALL( src ), dest_it, p );
}

template<class C1, class DestIt>
auto move( C1&& src, DestIt dest_it ) -> DestIt
{
	return std::move( MART_COMMON_ALL( src ), dest_it );
}

template<class R, class T>
void fill( R& range, const T& value )
{
	std::fill( MART_COMMON_ALL( range ), value );
}

template<class R, class T>
void fill_n( R& range, std::size_t n, const T& value )
{
	assert( n < range.size() );
	std::fill_n( range.begin(), n, value );
}

template<class R, class Generator>
void generate( R& range, Generator g )
{
	std::generate( MART_COMMON_ALL( range ), std::move( g ) );
}

template<class R, class Generator>
void generate_n( R& range, std::size_t n, Generator g )
{
	assert( range.size() >= n );
	std::generate_n( range.begin(), n, std::move( g ) );
}

} // namespace mart

#undef MART_COMMON_ALL

#endif
