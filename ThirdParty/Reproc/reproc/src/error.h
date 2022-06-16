#pragma once

#include <assert.h>

#define ASSERT(expression) assert(expression)

// Avoid unused assignment warnings in release mode when the result of an
// assignment is only used in an assert statement.
#define ASSERT_UNUSED(expression)                                              \
  do {                                                                         \
    (void) !(expression);                                                      \
    ASSERT((expression));                                                      \
  } while (0)

// Returns `r` if `expression` is false.
#define ASSERT_RETURN(expression, r)                                           \
  do {                                                                         \
    if (!(expression)) {                                                       \
      return (r);                                                              \
    }                                                                          \
  } while (0)

#define ASSERT_EINVAL(expression) ASSERT_RETURN(expression, REPROC_EINVAL)

const char *error_string(int error);
