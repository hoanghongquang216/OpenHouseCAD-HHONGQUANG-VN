#pragma once

#if defined(_MSC_VER)
#define OH_COMPILER_MSVC 1
#elif defined(__clang__)
#define OH_COMPILER_CLANG 1
#elif defined(__GNUC__)
#define OH_COMPILER_GCC 1
#else
#error Unsupported compiler
#endif
