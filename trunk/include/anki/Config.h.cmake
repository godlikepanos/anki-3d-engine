#ifndef ANKI_CONFIG_H
#define ANKI_CONFIG_H

#define ANKI_VERSION_MINOR ${ANKI_VERSION_MINOR}
#define ANKI_VERSION_MAJOR ${ANKI_VERSION_MAJOR}
#define ANKI_REVISION ${ANKI_REVISION}

#if defined(NDEBUG)
#	define ANKI_DEBUG !NDEBUG
#else
#	define ANKI_DEBUG 1
#endif

#define ANKI_FILE __FILE__
#define ANKI_FUNC __func__

// Operating system
#define ANKI_OS_LINUX 1 
#define ANKI_OS_ANDROID 2
#define ANKI_OS_APPLE 3
#define ANKI_OS_APPLE_IOS 4
#define ANKI_OS_WINDOWS 5

#if defined( __WIN32__ ) || defined( _WIN32 )
#	define ANKI_OS ANKI_OS_WINDOWS
#elif defined( __APPLE_CC__)
#	if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 40000 \
	|| __IPHONE_OS_VERSION_MIN_REQUIRED >= 40000
#		define ANKI_OS ANKI_OS_APPLE_IOS
#	else
#		define ANKI_OS ANKI_OS_APPLE
#	endif
#elif defined(__ANDROID__)
#	define ANKI_OS ANKI_OS_ANDROID
#else
#	define ANKI_OS ANKI_OS_LINUX
#endif

// POSIX system or not
#if ANKI_OS == ANKI_OS_LINUX || ANKI_OS == ANKI_OS_ANDROID \
	|| ANKI_OS == ANKI_OS_APPLE || ANKI_OS == ANKI_OS_APPLE_IOS
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

#define ANKI_MATH_SIMD_NONE 1
#define ANKI_MATH_SIMD_SSE 2
#define ANKI_MATH_SIMD_NEON 3
#define ANKI_MATH_SIMD ANKI_MATH_SIMD_${ANKI_MATH_SIMD}

#define ANKI_WINDOW_BACKEND_GLXX11 1
#define ANKI_WINDOW_BACKEND_EGLX11 2
#define ANKI_WINDOW_BACKEND_EGLFBDEV 3
#define ANKI_WINDOW_BACKEND ANKI_WINDOW_BACKEND_${ANKI_WINDOW_BACKEND}
#define ANKI_WINDOW_BACKEND_STR "ANKI_WINDOW_BACKEND_${ANKI_WINDOW_BACKEND}"

#define ANKI_GL_DESKTOP 1
#define ANKI_GL_ES 2
#if ANKI_WINDOW_BACKEND == ANKI_WINDOW_BACKEND_GLXX11
#	define ANKI_GL ANKI_GL_DESKTOP
#	define ANKI_GL_STR "ANKI_GL_DESKTOP"
#else
#	define ANKI_GL ANKI_GL_ES
#	define ANKI_GL_STR "ANKI_GL_ES"
#endif

#if defined(__GNUC__)
#	define ANKI_LIKELY(x) __builtin_expect((x), 1)
#	define ANKI_UNLIKELY(x) __builtin_expect((x), 0)
#	define ANKI_RESTRICT __restrict
#else
#	define ANKI_LIKELY(x) ((x) == 1)
#	define ANKI_UNLIKELY(x) ((x) == 1)
#	define ANKI_RESTRICT
#endif

// Workaround some GCC C++11 problems
#if ${ANKI_GCC_TO_STRING_WORKAROUND}
#	include <sstream>

namespace std {

template<typename T>
std::string to_string(const T x)
{
	stringstream ss;
	ss << x;
	return ss.str();
}

inline float stof(const string& str)
{
	stringstream ss(str);
	float f;
	ss >> f;
	return f;
}

inline int stoi(const string& str)
{
	stringstream ss(str);
	int i;
	ss >> i;
	return i;
}

} // end namespace std
#endif

#endif
