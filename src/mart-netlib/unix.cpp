#include "detail/dgram_socket_base_impl.hpp"
#include <mart-netlib/unix.hpp>

namespace mart::nw {
// classes related to the unix sockets protocol in general
namespace un {

} // namespace un
} // namespace mart::nw

namespace mart::nw::socks::detail {
template class DgramSocket<mart::nw::un::endpoint>;
}

