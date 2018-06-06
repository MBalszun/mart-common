#ifndef LIB_MART_COMMON_GUARD_ALGORITHMS_OUTPUT_RANGE_H
#define LIB_MART_COMMON_GUARD_ALGORITHMS_OUTPUT_RANGE_H
/**
 * OutputRange.h (mart-common/algorithms)
 *
 * Copyright (R) 2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
/* Proprietary Library Includes */
/* Standard Library Includes */
#include <iterator>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

class InfiniteRangeEndIteratorType {
};

// clang-format off
inline bool operator==( InfiniteRangeEndIteratorType, InfiniteRangeEndIteratorType ){ return true; }
inline bool operator!=( InfiniteRangeEndIteratorType, InfiniteRangeEndIteratorType ){ return false;}
template<class otherIt>	bool operator!=( otherIt, InfiniteRangeEndIteratorType ){ return true; }
template<class otherIt>	bool operator!=( InfiniteRangeEndIteratorType, otherIt ){ return true; }
template<class otherIt>	bool operator==( otherIt, InfiniteRangeEndIteratorType ){ return false; }
template<class otherIt>	bool operator==( InfiniteRangeEndIteratorType, otherIt ){ return false; }
// clang-format on

template<class BeginIt, class EndIt = BeginIt>
struct OutputRange {
	BeginIt _begin;
	EndIt   _end;

	constexpr BeginIt begin() const { return _begin; }
	constexpr EndIt   end() const { return _end; }

	static constexpr bool is_infinite() { return std::is_same<EndIt, InfiniteRangeEndIteratorType>::value; }

	static constexpr std::size_t infinite_length = ~std::size_t{0u};

	constexpr std::size_t size() const
	{
		if constexpr( is_infinite() ) {
			return infinite_length;
		} else {
			return std::distance( _begin, _end );
		}
	}
};

template<class Container>
auto back_inserter_range( Container& c )
{
	auto it = std::back_inserter( c );
	return OutputRange<decltype( it ), InfiniteRangeEndIteratorType>{it, {}};
}

namespace detail {
template<class C>
using OutputRange = ::mart::OutputRange<decltype( std::declval<C>().begin() ), decltype( std::declval<C>().end() )>;
}

} // namespace mart

#endif
