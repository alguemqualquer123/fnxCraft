#pragma once

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WINDOWS
#elif defined(__linux__)
	#define PLATFORM_LINUX
#elif defined(__APPLE__)
	#define PLATFORM_MACOS
#endif

// Compiler detection
#if defined(_MSC_VER)
	#define COMPILER_MSVC
#elif defined(__clang__)
	#define COMPILER_CLANG
#elif defined(__GNUC__)
	#define COMPILER_GCC
#endif

// Debug break intrinsic
#if defined(COMPILER_MSVC)
	#define DEBUG_BREAK() __debugbreak()
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
	#define DEBUG_BREAK() __builtin_trap()
#else
	#define DEBUG_BREAK() ((void)0)
#endif

// GPU preference for hybrid GPU laptops (NVIDIA Optimus / AMD PowerXpress)
#if defined(PLATFORM_WINDOWS) && defined(COMPILER_MSVC)
	#define EXPORT_GPU_PREFERENCE __declspec(dllexport)
#else
	#define EXPORT_GPU_PREFERENCE
#endif

// Platform-specific functions
#if defined(PLATFORM_WINDOWS)
	#define PLATFORM_sleep(ms) Sleep(ms)
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
	#include <unistd.h>
	#define PLATFORM_sleep(ms) usleep((ms) * 1000)
#endif
