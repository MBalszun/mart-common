#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_UNBLOCK_EXCPETION_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_MT_UNBLOCK_EXCPETION_H

#include <stdexcept>

namespace mart {
namespace experimental {
namespace mt {
struct Canceled : std::exception {

};
}
}
}

#endif
