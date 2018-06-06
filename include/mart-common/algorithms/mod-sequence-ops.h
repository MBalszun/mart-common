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
#include "./OutputRange.h"
/* Proprietary Library Includes */
/* Standard Library Includes */
#include <algorithm>
#include <cassert>
#include <iterator>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

// Will be undefined at end of file
#define MART_COMMON_ALL( rng ) std::begin( rng ), std::end( rng )

namespace mart {

template<class SrcRng, class Dest>
auto copy( const SrcRng& src, Dest&& dest ) -> detail::OutputRange<decltype( dest )>
{
	assert( src.size() <= dest.size() );
	return {std::copy( MART_COMMON_ALL( src ), dest.begin() ),dest.end()};
}

template<class SrcRng, class Dest, class Pred>
auto copy_if( const SrcRng& src, Dest&& dest, Pred p ) -> detail::OutputRange<decltype( dest )>
{
	assert( src.size() <= dest.size() );
	return {std::copy_if( MART_COMMON_ALL( src ), dest.begin(), p ), dest.end()};
}

template<class C1, class Dest>
auto move( C1& src, Dest&& dest ) -> detail::OutputRange<decltype( dest )>
{
	assert( src.size() <= dest.size() );
	return {std::move( MART_COMMON_ALL( src ), dest.begin() ), dest.end()};
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

// transform
template<class Src, class Dest, class UnaryOperation>
auto transform( const Src& src, Dest&& dest, UnaryOperation unary_op ) -> detail::OutputRange<decltype( dest )>
{
	assert( src.size() <= dest.size() );
	return {std::transform( MART_COMMON_ALL( src ), dest.begin(), unary_op ), dest.end()};
}

template<class Src1, class Src2, class Dest, class UnaryOperation>
auto transform( const Src1& src1, const Src2& src2, Dest& dest, UnaryOperation unary_op )
	-> detail::OutputRange<decltype( dest )>
{
	assert( src1.size() == src2.size() );
	assert( src1.size() <= dest.size() );
	return {std::transform( MART_COMMON_ALL( src1 ), src2.begin(), dest.begin(), unary_op ), dest.end()};
}

} // namespace mart

#undef MART_COMMON_ALL

#endif
