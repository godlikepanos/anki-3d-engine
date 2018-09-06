// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

/// @addtogroup config
/// @{

#define _ANKI_STR_HELPER(x) #x
#define _ANKI_STR(x) _ANKI_STR_HELPER(x)

#define ANKI_VERSION_MINOR ${ANKI_VERSION_MINOR}
#define ANKI_VERSION_MAJOR ${ANKI_VERSION_MAJOR}
#define ANKI_REVISION ${ANKI_REVISION}

#define ANKI_EXTRA_CHECKS ${_ANKI_EXTRA_CHECKS}
#define ANKI_DEBUG_SYMBOLS ${ANKI_DEBUG_SYMBOLS}
#define ANKI_OPTIMIZE ${ANKI_OPTIMIZE}
#define ANKI_TESTS ${ANKI_TESTS}

// Compiler
#define ANKI_COMPILER_CLANG 0
#define ANKI_COMPILER_GCC 1
#define ANKI_COMPILER_MSVC 2

#if defined(__clang__)
#	define ANKI_COMPILER ANKI_COMPILER_CLANG
#	define ANKI_COMPILER_STR "clang " __VERSION__
#elif defined(__GNUC__) || defined(__GNUG__)
#	define ANKI_COMPILER ANKI_COMPILER_GCC
#	define ANKI_COMPILER_STR "gcc " __VERSION__
#elif defined(_MSC_VER)
#	define ANKI_COMPILER ANKI_COMPILER_MSVC
#	define ANKI_COMPILER_STR "MSVC " _ANKI_STR(_MSC_FULL_VER)
#else
#	error Unknown compiler
#endif

// Operating system
#define ANKI_OS_LINUX 1
#define ANKI_OS_ANDROID 2
#define ANKI_OS_MACOS 3
#define ANKI_OS_IOS 4
#define ANKI_OS_WINDOWS 5

#if defined(__WIN32__) || defined(_WIN32)
#	define ANKI_OS ANKI_OS_WINDOWS
#elif defined(__APPLE_CC__)
#	if (defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
		&& __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 40000) \
	|| (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 40000)
#		define ANKI_OS ANKI_OS_IOS
#	else
#		define ANKI_OS ANKI_OS_MACOS
#	endif
#elif defined(__ANDROID__)
#	define ANKI_OS ANKI_OS_ANDROID
#else
#	define ANKI_OS ANKI_OS_LINUX
#endif

// POSIX system or not
#if ANKI_OS == ANKI_OS_LINUX || ANKI_OS == ANKI_OS_ANDROID || ANKI_OS == ANKI_OS_MACOS || ANKI_OS == ANKI_OS_IOS
#	define ANKI_POSIX 1
#else
#	define ANKI_POSIX 0
#endif

// CPU architecture
#define ANKI_CPU_ARCH_INTEL 1
#define ANKI_CPU_ARCH_ARM 2

#if defined(__GNUC__)
#	if defined(__arm__)
#		define ANKI_CPU_ARCH ANKI_CPU_ARCH_ARM
#		define ANKI_CPU_ARCH_STR "ANKI_CPU_ARCH_ARM"
#	elif defined(__i386__) || defined(__amd64__)
#		define ANKI_CPU_ARCH ANKI_CPU_ARCH_INTEL
#		define ANKI_CPU_ARCH_STR "ANKI_CPU_ARCH_INTEL"
#	else
#		error "Unknown CPU arch"
#	endif
#elif ANKI_COMPILER == ANKI_COMPILER_MSVC
#	if defined(_M_ARM)
#		define ANKI_CPU_ARCH ANKI_CPU_ARCH_ARM
#		define ANKI_CPU_ARCH_STR "ANKI_CPU_ARCH_ARM"
#	elif defined(_M_X64) || defined(_M_IX86)
#		define ANKI_CPU_ARCH ANKI_CPU_ARCH_INTEL
#		define ANKI_CPU_ARCH_STR "ANKI_CPU_ARCH_INTEL"
#	else
#		error "Unknown CPU arch"
#	endif
#else
#	error "Unsupported compiler"
#endif

// SIMD
#define ANKI_ENABLE_SIMD ${_ANKI_ENABLE_SIMD}

#define ANKI_SIMD_NONE 1
#define ANKI_SIMD_SSE 2
#define ANKI_SIMD_NEON 3

#if !ANKI_ENABLE_SIMD
#	define ANKI_SIMD ANKI_SIMD_NONE
#else
#	if ANKI_CPU_ARCH == ANKI_CPU_ARCH_INTEL
#		define ANKI_SIMD ANKI_SIMD_SSE
#	elif ANKI_CPU_ARCH == ANKI_CPU_ARCH_ARM
#		define ANKI_SIMD ANKI_SIMD_NEON
#	endif
#endif

// Window backend
#define ANKI_WINDOW_BACKEND_GLXX11 1
#define ANKI_WINDOW_BACKEND_EGLX11 2
#define ANKI_WINDOW_BACKEND_EGLFBDEV 3
#define ANKI_WINDOW_BACKEND_MACOS 4
#define ANKI_WINDOW_BACKEND_ANDROID 5
#define ANKI_WINDOW_BACKEND_SDL 6
#define ANKI_WINDOW_BACKEND ANKI_WINDOW_BACKEND_${ANKI_WINDOW_BACKEND}
#define ANKI_WINDOW_BACKEND_STR "ANKI_WINDOW_BACKEND_${ANKI_WINDOW_BACKEND}"

// OpenGL version
#define ANKI_GL_DESKTOP 1
#define ANKI_GL_ES 2
#if ANKI_OS == ANKI_OS_LINUX || ANKI_OS == ANKI_OS_MACOS || ANKI_OS == ANKI_OS_WINDOWS
#	define ANKI_GL ANKI_GL_DESKTOP
#	define ANKI_GL_STR "ANKI_GL_DESKTOP"
#else
#	define ANKI_GL ANKI_GL_ES
#	define ANKI_GL_STR "ANKI_GL_ES"
#endif

// Graphics backend
#define ANKI_GR_BACKEND_GL 1
#define ANKI_GR_BACKEND_VULKAN 2
#define ANKI_GR_BACKEND ANKI_GR_BACKEND_${ANKI_GR_BACKEND}

// Enable performance counters
#define ANKI_ENABLE_TRACE ${_ANKI_ENABLE_TRACE}

#define ANKI_FILE __FILE__
#define ANKI_FUNC __func__

// Some compiler struff
#if defined(__GNUC__)
#	define ANKI_LIKELY(x) __builtin_expect((x), 1)
#	define ANKI_UNLIKELY(x) __builtin_expect((x), 0)
#	define ANKI_RESTRICT __restrict
#	define ANKI_USE_RESULT __attribute__((warn_unused_result))
#	define ANKI_FORCE_INLINE __attribute__((always_inline))
#	define ANKI_DONT_INLINE __attribute__((noinline))
#	define ANKI_UNUSED __attribute__((__unused__))
#	define ANKI_COLD __attribute__((cold, optimize("Os")))
#	define ANKI_HOT __attribute__ ((hot))
#else
#	define ANKI_LIKELY(x) ((x) == 1)
#	define ANKI_UNLIKELY(x) ((x) == 1)
#	define ANKI_RESTRICT
#	define ANKI_USE_RESULT
#	define ANKI_FORCE_INLINE
#	define ANKI_DONT_INLINE
#	define ANKI_UNUSED
#	define ANKI_COLD
#	define ANKI_HOT
#endif

// Pack structs
#if defined(_MSC_VER)
#	define ANKI_BEGIN_PACKED_STRUCT __pragma(pack (push, 1))
#	define ANKI_END_PACKED_STRUCT __pragma(pack (pop))
#else
#	define ANKI_BEGIN_PACKED_STRUCT _Pragma("pack (push, 1)")
#	define ANKI_END_PACKED_STRUCT _Pragma("pack (pop)")
#endif

// Popcount
#ifdef _MSC_VER
#	include <intrin.h>
#	define __builtin_popcount __popcnt
#endif

#ifdef ANKI_BUILD
#	define anki_internal public
#else
#	define anki_internal protected
#endif

// General config
#define ANKI_SAFE_ALIGNMENT 16
#define ANKI_CACHE_LINE_SIZE 64
/// @}
