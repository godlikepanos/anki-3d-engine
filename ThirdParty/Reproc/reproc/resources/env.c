#include <stdio.h>

int main(int argc, const char **argv, const char **envp)
{
  (void) argc;
  (void) argv;

  for (size_t i = 0; envp[i] != NULL; i++) {
    printf("%s\n", envp[i]);
  }

  return 0;
}
