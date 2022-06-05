#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0600 // _WIN32_WINNT_VISTA
#elif _WIN32_WINNT < 0x0600
  #error "_WIN32_WINNT must be greater than _WIN32_WINNT_VISTA (0x0600)"
#endif

#include "handle.h"

#include <windows.h>

#include "error.h"

const HANDLE HANDLE_INVALID = INVALID_HANDLE_VALUE; // NOLINT

// `handle_cloexec` is POSIX-only.

HANDLE handle_destroy(HANDLE handle)
{
  if (handle == NULL || handle == HANDLE_INVALID) {
    return HANDLE_INVALID;
  }

  int r = CloseHandle(handle);
  ASSERT_UNUSED(r != 0);

  return HANDLE_INVALID;
}
