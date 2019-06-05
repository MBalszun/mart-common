#ifndef LIB_MART_COMMON_GUARD_STRING_VIEW_OSTREAM_H
#define LIB_MART_COMMON_GUARD_STRING_VIEW_OSTREAM_H
/**
 * StringViewOstream.h (mart-common)
 *
 * Copyright (C) 2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief: Provides operator<< for mart::StringView
 *
 */
#include "StringView.h"

#include <ostream>

namespace mart {
inline std::ostream& operator<<( std::ostream& out, const StringView string )
{
	out.write( string.data(), string.size() );
	return out;
}
} // namespace mart

#endif