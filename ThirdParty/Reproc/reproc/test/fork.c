#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <reproc/run.h>

#include "assert.h"

int main(void)
{
  reproc_t *process = reproc_new();
  const char *MESSAGE = "reproc stands for REdirected PROCess!";
  char *output = NULL;
  reproc_sink sink = reproc_sink_string(&output);
  int r = -1;

  r = reproc_start(process, NULL, (reproc_options){ .fork = true });

  if (r == 0) {
    printf("%s", MESSAGE);
    fclose(stdout); // `_exit` doesn't flush stdout.
    _exit(EXIT_SUCCESS);
  }

  ASSERT_OK(r);

  r = reproc_drain(process, sink, sink);
  ASSERT_OK(r);

  ASSERT(output != NULL);
  ASSERT_EQ_STR(output, MESSAGE);

  r = reproc_wait(process, REPROC_INFINITE);
  ASSERT_OK(r);

  reproc_destroy(process);

  reproc_free(output);
}
