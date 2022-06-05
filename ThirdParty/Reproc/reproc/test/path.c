#include <reproc/run.h>

#include "assert.h"

int main(void)
{
  const char *argv[] = { RESOURCE_DIRECTORY "/path", NULL };
  int r = -1;

  r = reproc_run(argv, (reproc_options){ .redirect.path = "path.txt" });
  ASSERT_OK(r);

  FILE *file = fopen("path.txt", "rb");
  ASSERT(file != NULL);

  r = fseek(file, 0, SEEK_END);
  ASSERT_OK(r);

  r = (int) ftell(file);
  ASSERT_OK(r);

  size_t size = (size_t) r;
  char *string = malloc(size + 1);
  ASSERT(string != NULL);

  rewind(file);
  r = (int) fread(string, sizeof(char), size, file);
  ASSERT_EQ_INT(r, (int) size);

  string[r] = '\0';

  r = fclose(file);
  ASSERT_OK(r);

  r = remove("path.txt");
  ASSERT_OK(r);

  ASSERT_EQ_STR(string, argv[0]);

  free(string);
}
