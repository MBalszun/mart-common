#ifndef LIB_MART_COMMON_GUARD_EXCEPTIONS_H
#define LIB_MART_COMMON_GUARD_EXCEPTIONS_H

/**
 * exceptions.h (mart-common)
 *
 * Copyright (C) 2016-2020: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@tum.de>
 * @brief: Custom runtime exceptions that accept a mba::im_zstr
 *
 */

#include <im_str/im_str.hpp>

#include <exception>

namespace mart {

class RuntimeError : public std::exception {
	mba::im_zstr _msg;

public:
	RuntimeError( mba::im_zstr message ) noexcept
		: _msg( std::move( message ) )
	{
	}
	const char* what() const noexcept override { return _msg.c_str(); }
};

struct InvalidArgument : RuntimeError {
	using RuntimeError::RuntimeError;
};

struct BlockingOpCanceled : RuntimeError {
	BlockingOpCanceled() noexcept
		: RuntimeError( mba::im_zstr( "Blocking operation was canceled" ) )
	{
	}
	using RuntimeError::RuntimeError;
};

} // namespace mart

#endif