#include <stdio.h>

#if defined(_WIN32)
  #include <direct.h>
  #define getcwd _getcwd
#else
  #include <unistd.h>
#endif

int main()
{
  char working_directory[8096];

  if (getcwd(working_directory, sizeof(working_directory)) == NULL) {
    return 1;
  }

  printf("%s", working_directory);

  return 0;
}
