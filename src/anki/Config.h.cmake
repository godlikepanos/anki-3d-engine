// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

/// @addtogroup config
/// @{

#define ANKI_VERSION_MINOR ${ANKI_VERSION_MINOR}
#define ANKI_VERSION_MAJOR ${ANKI_VERSION_MAJOR}
#define ANKI_REVISION ${ANKI_REVISION}

#define ANKI_EXTRA_CHECKS ${_ANKI_EXTRA_CHECKS}
#define ANKI_DEBUG_SYMBOLS ${ANKI_DEBUG_SYMBOLS}
#define ANKI_OPTIMIZE ${ANKI_OPTIMIZE}

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
#	define ANKI_UNUSED __attribute__((__unused__))
#else
#	define ANKI_LIKELY(x) ((x) == 1)
#	define ANKI_UNLIKELY(x) ((x) == 1)
#	define ANKI_RESTRICT
#	define ANKI_USE_RESULT
#	define ANKI_FORCE_INLINE
#	define ANKI_UNUSED
#endif

#ifdef ANKI_BUILD
#	define anki_internal public
#else
#	define anki_internal protected
#endif

// General config
#define ANKI_SAFE_ALIGNMENT 16
/// @}
