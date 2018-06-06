#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_NUMERIC_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_NUMERIC_H
/**
 * numeric.h (mart-common/algorithms)
 *
 * Copyright (R) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides wrappers around standard numeric algorithms
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
/* Proprietary Library Includes */
/* Standard Library Includes */
#include <iterator>
#include <numeric>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

// Will be undefined at end of file
#define MART_COMMON_ALL( rng ) std::begin( rng ), std::end( rng )

namespace mart {

template<class Src, class T>
T accumulate( const Src& src, T init )
{
	return std::accumulate( MART_COMMON_ALL( src ), init );
}

template<class Src, class T, class BinaryOperation>
T accumulate( const Src& src, T init, BinaryOperation op )
{
	return std::accumulate( MART_COMMON_ALL( src ), init, op );
}

template<class Src1, class Src2, class T>
T inner_product( const Src1& src1, const Src2& src2, T value )
{
	asssert( src1.size() <= src2.size() );
	return std::inner_product( MART_COMMON_ALL( src1 ), src2.begin(), value );
}

template<class Src1, class Src2, class T, class BinaryOp1, class BinaryOp2>
T inner_product( const Src1& src1, const Src2& src2, T value, BinaryOp1 op1, BinaryOp2 op2 )
{
	asssert( src1.size() <= src2.size() );
	return std::inner_product( MART_COMMON_ALL( src1 ), src2.begin(), value, op1, op2 );
}

//template<class Src, class Dest>
//auto adjacent_difference( const Src& src, Dest& dest ) -> decltype( dest.begin() )
//{
//	return std::adjacent_difference( MART_COMMON_ALL( src ), dest.begin() );
//}
//
//template<class Src, class Dest, class BinaryOp>
//auto adjacent_difference( const Src& src, Dest& dest, BinaryOp op ) -> decltype( dest.begin() )
//{
//	return std::adjacent_difference( MART_COMMON_ALL( src ), dest.begin(), op );
//}

} // namespace mart

#undef MART_COMMON_ALL

#endif
