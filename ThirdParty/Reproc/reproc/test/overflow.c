#include <reproc/run.h>

#include "assert.h"

int main(void)
{
  const char *argv[] = { RESOURCE_DIRECTORY "/overflow", NULL };
  char *output = NULL;
  reproc_sink sink = reproc_sink_string(&output);
  int r = -1;

  r = reproc_run_ex(argv, (reproc_options){ 0 }, sink, sink);
  ASSERT_OK(r);
  ASSERT(output != NULL);

  reproc_free(output);
}
