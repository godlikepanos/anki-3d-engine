#ifndef ANKI_UTIL_PLATFORM_H
#define ANKI_UTIL_PLATFORM_H


//
// Compiler
//

#define ANKI_COMPILER_UNKNOWN 0
#define ANKI_COMPILER_GCC 1
#define ANKI_COMPILER_CLANG 2
#define ANKI_COMPILER_INTEL 3
#define ANKI_COMPILER_OPEN64 4
#define ANKI_COMPILER_MICROSOFT 5


#if defined(__GNUC__)
#	if defined(__clang__)
#		define ANKI_COMPILER ANKI_COMPILER_CLANG
#	else
#		define ANKI_COMPILER ANKI_COMPILER_GCC
#	endif
#else
#	define ANKI_COMPILER ANKI_COMPILER_UNKNOWN
#endif

//
// Platform
//

#define ANKI_PLATFORM_UNKNOWN 0
#define ANKI_PLATFORM_LINUX 1
#define ANKI_PLATFORM_WINDOWS 2
#define ANKI_PLATFORM_APPLE 3


#if defined(__gnu_linux__)
#	define ANKI_PLATFORM ANKI_PLATFORM_LINUX
#elif defined(__WIN32__) || defined(_WIN32)
#	define ANKI_PLATFORM ANKI_PLATFORM_WINDOWS
#elif defined(__APPLE_CC__)
#	define ANKI_PLATFORM ANKI_PLATFORM_APPLE
#else
#	define ANKI_PLATFORM ANKI_PLATFORM_UNKNOWN
#endif

//
// Arch
//

#define ANKI_ARCH_UNKNOWN 0
#define ANKI_ARCH_X86_32 1
#define ANKI_ARCH_X86_64 2

/// XXX


#endif
