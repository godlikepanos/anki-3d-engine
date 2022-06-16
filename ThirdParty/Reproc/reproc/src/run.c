#include <reproc/run.h>

#include <reproc/drain.h>

#include "error.h"

int reproc_run(const char *const *argv, reproc_options options)
{
  if (!options.redirect.discard && !options.redirect.file &&
      !options.redirect.path) {
    options.redirect.parent = true;
  }

  return reproc_run_ex(argv, options, REPROC_SINK_NULL, REPROC_SINK_NULL);
}

int reproc_run_ex(const char *const *argv,
                  reproc_options options, // lgtm [cpp/large-parameter]
                  reproc_sink out,
                  reproc_sink err)
{
  reproc_t *process = NULL;
  int r = REPROC_ENOMEM;

  // There's no way for `reproc_run_ex` to inform the caller whether we're in
  // the forked process or the parent process so let's not allow forking when
  // using `reproc_run_ex`.
  ASSERT_EINVAL(!options.fork);

  process = reproc_new();
  if (process == NULL) {
    goto finish;
  }

  r = reproc_start(process, argv, options);
  if (r < 0) {
    goto finish;
  }

  r = reproc_drain(process, out, err);
  if (r < 0) {
    goto finish;
  }

  r = reproc_stop(process, options.stop);
  if (r < 0) {
    goto finish;
  }

finish:
  reproc_destroy(process);

  return r;
}
