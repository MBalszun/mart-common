#include <exception>
#include <mart-common/ConstString.h>

namespace mart::nw {
struct generic_nw_error : std::exception {
	generic_nw_error( mart::ConstString message )
		: _message( std::move(message).createZStr() )
	{
	}

	const char* what() const noexcept override { return _message.c_str();
	}

	mart::ConstString _message;
};
} // namespace mart::nw