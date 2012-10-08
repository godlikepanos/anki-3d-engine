#ifndef ANKI_UTIL_PLATFORM_H
#define ANKI_UTIL_PLATFORM_H

//
// Compiler
//

#if defined(__GNUC__)
#	if defined(__clang__)
#		define ANKI_COMPILER_CLANG 1
#	else
#		define ANKI_COMPILER_GCC 1
#	endif
#else
#	define ANKI_COMPILER_UNKNOWN
#endif

//
// Platform
//

#if defined(__gnu_linux__)
#	define ANKI_PLATFORM_LINUX 1
#elif defined(__WIN32__) || defined(_WIN32)
#	define ANKI_PLATFORM_WINDOWS 1
#elif defined(__APPLE_CC__)
#	define ANKI_PLATFORM_APPLE 1
#else
#	define ANKI_PLATFORM_UNKNOWN 1
#endif

#endif
