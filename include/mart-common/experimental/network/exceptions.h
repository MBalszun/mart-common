#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_EXCEPTIONS_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_EXCEPTIONS_H

#include <exception>

#include "../../ConstString.h"

namespace mart::experimental::nw {

class RuntimeError : public std::exception {
	mba::im_zstr _msg;

public:
	RuntimeError( mba::im_zstr message )
		: _msg( std::move( message ) )
	{
	}
	const char* what() const noexcept override { return _msg.c_str(); }
};

struct InvalidArgument : RuntimeError {
	using RuntimeError::RuntimeError;
};

} // namespace mart::experimental::nw

#endif