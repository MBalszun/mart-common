#include <exception>
#include <mart-common/ConstString.h>

namespace mart::nw {

struct nw_error : std::exception {
protected:
	nw_error() = default;
};

struct generic_nw_error_base : nw_error {

	const char* what() const noexcept override { return _message.c_str(); }
	generic_nw_error_base( mart::ConstString message )
		: _message( std::move( message ).createZStr() )
	{
	}
	mart::ConstString _message;
};


struct generic_nw_error final : generic_nw_error_base {
	generic_nw_error_base::generic_nw_error_base;
};


struct invalid_address_string final : generic_nw_error_base {
	using generic_nw_error_base::generic_nw_error_base;
};
} // namespace mart::nw