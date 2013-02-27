#ifndef ANKI_CONFIG_H
#define ANKI_CONFIG_H

#define ANKI_VERSION_MINOR ${ANKI_VERSION_MINOR}
#define ANKI_VERSION_MAJOR ${ANKI_VERSION_MAJOR}
#define ANKI_REVISION ${ANKI_REVISION}

#define ANKI_MATH_SIMD_NONE 1
#define ANKI_MATH_SIMD_SSE 2
#define ANKI_MATH_SIMD_NEON 3
#define ANKI_MATH_SIMD ANKI_MATH_SIMD_${ANKI_MATH_SIMD}

#define ANKI_WINDOW_BACKEND_GLXX11 1
#define ANKI_WINDOW_BACKEND_EGLX11 2
#define ANKI_WINDOW_BACKEND_EGLFBDEV 3
#define ANKI_WINDOW_BACKEND ANKI_WINDOW_BACKEND_${ANKI_WINDOW_BACKEND}

#define ANKI_GL_DESKTOP 1
#define ANKI_GL_ES 2
#if ANKI_WINDOW_BACKEND == ANKI_WINDOW_BACKEND_GLXX11
#	define ANKI_GL ANKI_GL_DESKTOP
#else
#	define ANKI_GL ANKI_GL_ES
#endif

#if defined(NDEBUG)
#	define ANKI_DEBUG !NDEBUG
#else
#	define ANKI_DEBUG 1
#endif

#define ANKI_FILE __FILE__
#define ANKI_FUNC __func__

#if defined(__GNUC__)
#	define ANKI_LIKELY(x) __builtin_expect((x), 1)
#	define ANKI_UNLIKELY(x) __builtin_expect((x), 0)
#	define ANKI_RESTRICT __restrict
#else
#	define ANKI_LIKELY(x) ((x) == 1)
#	define ANKI_UNLIKELY(x) ((x) == 1)
#	define ANKI_RESTRICT
#endif

#endif
