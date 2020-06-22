#ifndef IM_STR_CONFIG_HPP
#define IM_STR_CONFIG_HPP

// clang format doesn't indent neste #if blocks, so lets do it manually
// clang-format off

// In c++20, our destructor can be declared constexpr
// that then propagetes to many member functions of im_str.
#ifndef IM_STR_USE_CONSTEXPR_DESTRUCTOR
	#if __cpp_constexpr >= 201907
		#define IM_STR_USE_CONSTEXPR_DESTRUCTOR 1
		#define IM_STR_CONSTEXPR_IN_CPP_20 constexpr
	#else
		#define IM_STR_USE_CONSTEXPR_DESTRUCTOR 0
		#define IM_STR_CONSTEXPR_IN_CPP_20
	#endif
#endif // !IM_STR_USE_CONSTEXPR_DESTRUCTOR


// Handle custom allocation logic via pmr memory_resource
#ifndef IM_STR_USE_ALLOC

	// default to no and overwrite later if appropriate
	#define IM_STR_USE_ALLOC 0

	#if __has_include( <memory_resource>)
		#include <memory_resource>
		#if defined( __cpp_lib_memory_resource ) && __cpp_lib_memory_resource >= 201603L

			#undef IM_STR_USE_ALLOC
			#define IM_STR_USE_ALLOC 1

		#endif
	#endif

#endif


#ifndef IM_STR_USE_CUSTOM_DYN_ARRAY
	#define IM_STR_USE_CUSTOM_DYN_ARRAY	1
#else
	#define IM_STR_USE_CUSTOM_DYN_ARRAY	0
#endif
// clang-format on

#endif