#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT(expression) ASSERT_MSG(expression, "%s", "")
#define ASSERT_OK(r) ASSERT_MSG(r >= 0, "%s", reproc_strerror(r))

#define ASSERT_EQ_MEM(left, right, size)                                       \
  ASSERT_MSG(memcmp(left, right, size) == 0, "\"%.*s\" == \"%.*s\"",           \
             (int) size, left, (int) size, right)

#define ASSERT_EQ_STR(left, right)                                             \
  ASSERT_MSG(strcmp(left, right) == 0, "%s == %s", left, right)

#define ASSERT_GE_SIZE(left, right)                                            \
  ASSERT_MSG(left >= right, "%zu >= %zu", left, right)

#define ASSERT_EQ_SIZE(left, right)                                            \
  ASSERT_MSG(left == right, "%zu == %zu", left, right)

#define ASSERT_EQ_INT(left, right)                                             \
  ASSERT_MSG(left == right, "%i == %i", left, right)

#ifdef _WIN32
  #define ABORT() exit(EXIT_FAILURE)
#else
  // Use `abort` so we get a coredump.
  #define ABORT() abort()
#endif

#define ASSERT_MSG(expression, format, ...)                                    \
  do {                                                                         \
    if (!(expression)) {                                                       \
      fprintf(stderr, "%s:%u: Assertion '%s' (" format ") failed", __FILE__,   \
              __LINE__, #expression, __VA_ARGS__);                             \
                                                                               \
      fflush(stderr);                                                          \
                                                                               \
      ABORT();                                                                 \
    }                                                                          \
  } while (0)
