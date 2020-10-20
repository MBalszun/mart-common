#ifndef LIB_MART_COMMON_GUARD_NW_UDP_H
#define LIB_MART_COMMON_GUARD_NW_UDP_H
/**
 * udp.h (mart-common/nw)
 *
 * Copyright (C) 2015-2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	This file provides a simple udp socket implementation
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
#include "ip.hpp"

#include "detail/socket_base.hpp"

/* Proprietary Library Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart::nw {
namespace ip::udp {

using endpoint = ip::basic_endpoint_v4<mart::nw::ip::TransportProtocol::Udp>;

constexpr std::optional<endpoint> try_parse_v4_endpoint( std::string_view str ) noexcept
{
	return mart::nw::ip::try_parse_v4_endpoint<mart::nw::ip::TransportProtocol::Udp>( str );
}

} // namespace ip::udp
} // namespace mart::nw

namespace mart::nw::socks::detail {
extern template class DgramSocket<mart::nw::ip::udp::endpoint>;
}

namespace mart::nw {
namespace ip::udp {
using Socket = mart::nw::socks::detail::DgramSocket<endpoint>;
} // namespace ip::udp
} // namespace mart::nw

#endif