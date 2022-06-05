#define _POSIX_C_SOURCE 200809L

#include "error.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <reproc/reproc.h>

#include "macro.h"

const int REPROC_EINVAL = -EINVAL;
const int REPROC_EPIPE = -EPIPE;
const int REPROC_ETIMEDOUT = -ETIMEDOUT;
const int REPROC_ENOMEM = -ENOMEM;
const int REPROC_EWOULDBLOCK = -EWOULDBLOCK;

enum { ERROR_STRING_MAX_SIZE = 512 };

const char *error_string(int error)
{
  static THREAD_LOCAL char string[ERROR_STRING_MAX_SIZE];

  int r = strerror_r(abs(error), string, ARRAY_SIZE(string));
  if (r != 0) {
    return "Failed to retrieve error string";
  }

  return string;
}
