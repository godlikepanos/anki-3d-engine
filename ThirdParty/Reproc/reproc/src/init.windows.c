#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0600 // _WIN32_WINNT_VISTA
#elif _WIN32_WINNT < 0x0600
  #error "_WIN32_WINNT must be greater than _WIN32_WINNT_VISTA (0x0600)"
#endif

#include "init.h"

#include <winsock2.h>

#include "error.h"

int init(void)
{
  WSADATA data;
  int r = WSAStartup(MAKEWORD(2, 2), &data);
  return -r;
}

void deinit(void)
{
  int saved = WSAGetLastError();

  int r = WSACleanup();
  ASSERT_UNUSED(r == 0);

  WSASetLastError(saved);
}
