#pragma once

#include "Compiler.hpp"

#if defined(OH_COMPILER_MSVC)
#define OH_FORCE_INLINE __forceinline
#elif defined(OH_COMPILER_GCC) || defined(OH_COMPILER_CLANG)
#define OH_FORCE_INLINE inline __attribute__((always_inline))
#else
#define OH_FORCE_INLINE inline
#endif
