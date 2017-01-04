/*
* basic_types.h
*
*  Created on: 2016-09-29
*      Author: Michael Balszun <michael.balszun@tum.de>
*
*      This file provides some basic types used in networking
*      Currently (2016-09-29) this is mainly about network vs host order integer types
*/

#ifndef LIBS_MART_COMMON_EXPERIMENTAL_BASIC_TYPES_H_
#define LIBS_MART_COMMON_EXPERIMENTAL_BASIC_TYPES_H_

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <cstdint>
#include <chrono>

/* Proprietary Library Includes */

/* Project Includes */
#include "../../utils.h"

#include "port_layer.h"

namespace mart {
namespace experimental {
namespace nw {

//Make unsigned interger in host and network order distinct types
enum class uint64_net_t : uint64_t {};
enum class uint32_net_t : uint32_t {};
enum class uint16_net_t : uint16_t {};

using uint64_host_t = uint64_t;
using uint32_host_t = uint32_t;
using uint16_host_t = uint16_t;

/*#### implement portable byte swap function that compiles down to a single bswap instruction on any platform I've tested so far #########*/
namespace _impl_detail_bs {
//create a bitmasks of type t, where either bits [0...N/2) or [N/2...N) are set to one

template<class T> constexpr T lb_mask(unsigned int N = sizeof(T)) { return (T(0x1) << (N * 8 / 2 )) - T(0x1); }
template<class T> constexpr T hb_mask(unsigned int N = sizeof(T)) { return lb_mask<T>(N) << N * 8 / 2 ; }

template<class T, size_t N = sizeof(T)>
struct mbswap {
	static_assert(N % 2 == 0, "bswap Works only for types whose size is a power of 2");
	static constexpr T calc(T d)
	{
		return mbswap<T, N / 2>::calc(d >> (N * 8 / 2) & lb_mask<T>(N)) | (mbswap<T, N / 2>::calc(d) << (N * 8 / 2) & hb_mask<T>(N));
	}
};

template<class T>
struct mbswap<T, 1> {
	static constexpr T calc(T d)
	{
		return d;
	}
};

} //_impl_detail_bs

template<class T, size_t N = sizeof(T)>
static constexpr T bswap(T d)
{
	return _impl_detail_bs::mbswap<T, N>::calc(d);
}

#if MBA_BYTE_ORDER == MBA_ORDER_LITTLE_ENDIAN //use whatever compiletime mechanism available to determine, if net byte order (big endian) is the same / or different than machine order

constexpr uint16_net_t  to_net_order(uint16_host_t host_rep) { return uint16_net_t(bswap(host_rep)); }
constexpr uint32_net_t  to_net_order(uint32_host_t host_rep) { return uint32_net_t(bswap(host_rep)); }
constexpr uint64_net_t  to_net_order(uint64_host_t host_rep) { return uint64_net_t(bswap(host_rep)); }

constexpr uint16_host_t to_host_order(uint16_net_t net_rep) { return uint16_host_t(bswap(static_cast<uint16_t>(net_rep))); }
constexpr uint32_host_t to_host_order(uint32_net_t net_rep) { return uint32_host_t(bswap(static_cast<uint32_t>(net_rep))); }
constexpr uint64_host_t to_host_order(uint64_net_t net_rep) { return uint64_host_t(bswap(static_cast<uint64_t>(net_rep))); }

#else

constexpr uint16_net_t  to_net_order(uint16_host_t host_rep) { return uint16_net_t(host_rep); }
constexpr uint32_net_t  to_net_order(uint32_host_t host_rep) { return uint32_net_t(host_rep); }
constexpr uint64_net_t  to_net_order(uint64_host_t host_rep) { return uint64_net_t(host_rep); }

constexpr uint16_host_t to_host_order(uint16_net_t net_rep) { return uint16_host_t(net_rep); }
constexpr uint32_host_t to_host_order(uint32_net_t net_rep) { return uint32_host_t(net_rep); }
constexpr uint64_host_t to_host_order(uint64_net_t net_rep) { return uint64_host_t(net_rep); }

#endif

} //experimental
} //nw
} //mart

#endif