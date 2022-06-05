#include <reproc/reproc.h>

#include "assert.h"

static void stop(REPROC_STOP action, int status)
{
  int r = -1;

  reproc_t *process = reproc_new();
  ASSERT(process);

  const char *argv[] = { RESOURCE_DIRECTORY "/stop", NULL };

  r = reproc_start(process, argv, (reproc_options){ 0 });
  ASSERT_OK(r);

  r = reproc_wait(process, 50);
  ASSERT(r == REPROC_ETIMEDOUT);

  reproc_stop_actions stop = { .first = { action, 500 } };

  r = reproc_stop(process, stop);
  ASSERT_EQ_INT(r, status);

  reproc_destroy(process);
}

int main(void)
{
  stop(REPROC_STOP_TERMINATE, REPROC_SIGTERM);
  stop(REPROC_STOP_KILL, REPROC_SIGKILL);
}
