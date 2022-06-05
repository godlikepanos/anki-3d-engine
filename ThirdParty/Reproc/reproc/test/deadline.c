#include <reproc/run.h>

#include "assert.h"

int main(void)
{
  const char *argv[] = { RESOURCE_DIRECTORY "/deadline", NULL };
  int r = reproc_run(argv, (reproc_options){ .deadline = 100 });
  ASSERT(r == REPROC_SIGTERM);
}
