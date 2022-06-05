#include <stdio.h>

int main(void)
{
  char input[8096];

  if (fgets(input, sizeof(input), stdin) == NULL) {
    return 1;
  }

  fprintf(stdout, "%s", input);
  fprintf(stderr, "%s", input);

  return 0;
}
