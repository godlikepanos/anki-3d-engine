#include <reproc/run.h>

#include "assert.h"

static void replace(char *string, char old, char new)
{
  for (size_t i = 0; i < strlen(string); i++) {
    string[i] = (char) (string[i] == old ? new : string[i]);
  }
}

int main(void)
{
  const char *argv[] = { RESOURCE_DIRECTORY "/working-directory", NULL };
  char *output = NULL;
  reproc_sink sink = reproc_sink_string(&output);
  int r = -1;

  r = reproc_run_ex(argv,
                    (reproc_options){ .working_directory = RESOURCE_DIRECTORY },
                    sink, sink);
  ASSERT_OK(r);
  ASSERT(output != NULL);

  replace(output, '\\', '/');
  ASSERT_EQ_STR(output, RESOURCE_DIRECTORY);

  reproc_free(output);
}
