#pragma once

#ifdef _WIN32
  #include <windows.h>
static inline void millisleep(long ms)
{
  Sleep((DWORD) ms);
}
#else
  #define _POSIX_C_SOURCE 200809L
  #include <time.h>
static inline void millisleep(long ms)
{
  nanosleep(&(struct timespec){ .tv_sec = (ms) / 1000,
                                .tv_nsec = ((ms) % 1000L) * 1000000 },
            NULL);
}
#endif
