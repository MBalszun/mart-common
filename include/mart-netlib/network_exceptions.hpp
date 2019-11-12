#ifndef LIB_MART_COMMON_GUARD_NW_NETWORK_EXCEPTIONS_H
#define LIB_MART_COMMON_GUARD_NW_NETWORK_EXCEPTIONS_H
/**
 * network exceptions.hpp (mart-netlib)
 *
 * Copyright (C) 2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@tum.de>
 * @brief:  Provides exception definitions for usage in network types
 *
 */

#include <exception>
#include <im_str/im_str.hpp>

namespace mart::nw {

struct nw_error : std::exception {
protected:
	nw_error() = default;
};

struct generic_nw_error_base : nw_error {

	const char* what() const noexcept override { return _message.c_str(); }
	generic_nw_error_base( mba::im_zstr message )
		: _message( std::move( message ) )
	{
	}
	mba::im_zstr _message;
};


struct generic_nw_error final : generic_nw_error_base {
	using generic_nw_error_base::generic_nw_error_base;
};


struct invalid_address_string final : generic_nw_error_base {
	using generic_nw_error_base::generic_nw_error_base;
};
} // namespace mart::nw
#endif