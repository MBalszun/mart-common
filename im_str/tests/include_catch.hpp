#pragma once
/*
 * Purpose of this header is to have a central place
 * to include the catch library header, but with various
 * diagnostics disabled.
 * TODO: Find a way to achieve this from the tool invocation
 */

#ifdef _MSC_VER

#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <catch2/catch.hpp>
#pragma warning( pop )

#else

#include <catch2/catch.hpp>

#endif // _MSC_VER