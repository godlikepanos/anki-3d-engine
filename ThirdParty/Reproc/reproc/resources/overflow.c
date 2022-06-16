#include <stdio.h>
#include <stdlib.h>

int main()
{
  char buffer[8192];

  for (int i = 0; i < 200; i++) {
    FILE *stream = rand() % 2 ? stdout : stderr; // NOLINT
    fprintf(stream, "%s", buffer);
  }

  return 0;
}
