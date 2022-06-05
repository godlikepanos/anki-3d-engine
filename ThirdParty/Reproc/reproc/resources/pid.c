#include <stdio.h>

#ifdef _WIN32
  #include <Windows.h>
  #define getpid (int) GetCurrentProcessId
#else
  #include <unistd.h>
#endif

int main(void)
{
  printf("%d", getpid());

  return 0;
}
