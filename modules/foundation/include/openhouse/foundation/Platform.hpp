#pragma once

#if defined(_WIN32)
#define OH_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#define OH_PLATFORM_LINUX 1
#elif defined(__APPLE__)
#define OH_PLATFORM_MACOS 1
#else
#error Unsupported platform
#endif
