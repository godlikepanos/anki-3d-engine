#include <reproc/run.h>

#include "assert.h"

int main(void)
{
  const char *argv[] = { RESOURCE_DIRECTORY "/env", NULL };
  const char *envp[] = { "IP=127.0.0.1", "PORT=8080", NULL };
  char *output = NULL;
  reproc_sink sink = reproc_sink_string(&output);
  int r = -1;

  r = reproc_run_ex(argv,
                    (reproc_options){ .env.behavior = REPROC_ENV_EMPTY,
                                      .env.extra = envp },
                    sink, sink);
  ASSERT_OK(r);
  ASSERT(output != NULL);

  const char *current = output;

  for (size_t i = 0; i < 2; i++) {
    size_t size = strlen(envp[i]);

    ASSERT_GE_SIZE(strlen(current), size);
    ASSERT_EQ_MEM(current, envp[i], size);

    current += size;
    current += *current == '\r';
    current += *current == '\n';
  }

  ASSERT_EQ_SIZE(strlen(current), (size_t) 0);

  reproc_free(output);
}
