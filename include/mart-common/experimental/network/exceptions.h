#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_EXCEPTIONS_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_EXCEPTIONS_H

#include <exception>

#include "../../ConstString.h"

namespace mart::experimental::nw {

class RuntimeError : public std::exception {
	mart::ConstString _msg;

public:
	RuntimeError( mart::ConstString message )
		: _msg( std::move( message ).createZStr() )
	{
	}
	const char* what() const noexcept override { return _msg.c_str(); }
};

struct InvalidArgument : RuntimeError {
	using RuntimeError::RuntimeError;
};

} // namespace mart::experimental::nw

#endif