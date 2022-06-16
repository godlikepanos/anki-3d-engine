#define _POSIX_C_SOURCE 200809L

#include "clock.h"

#include <time.h>

#include "error.h"

int64_t now(void)
{
  struct timespec timespec = { 0 };

  int r = clock_gettime(CLOCK_REALTIME, &timespec);
  ASSERT_UNUSED(r == 0);

  return timespec.tv_sec * 1000 + timespec.tv_nsec / 1000000;
}
