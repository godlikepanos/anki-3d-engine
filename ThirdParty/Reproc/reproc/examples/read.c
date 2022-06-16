#include <stdlib.h>
#include <string.h>

#include <reproc/reproc.h>

// Prints the output of the given command using `reproc_read`. Usually, using
// `reproc_run` or `reproc_drain` is a better solution when dealing with a
// single child process.
int main(int argc, const char **argv)
{
  (void) argc;

  // `reproc_t` stores necessary information between calls to reproc's API.
  reproc_t *process = NULL;
  char *output = NULL;
  size_t size = 0;
  int r = REPROC_ENOMEM;

  process = reproc_new();
  if (process == NULL) {
    goto finish;
  }

  // `reproc_start` takes a child process instance (`reproc_t`), argv and
  // a set of options including the working directory and environment of the
  // child process. If the working directory is `NULL` the working directory of
  // the parent process is used. If the environment is `NULL`, the environment
  // of the parent process is used.
  r = reproc_start(process, argv + 1, (reproc_options){ 0 });

  // On failure, reproc's API functions return a negative `errno` (POSIX) or
  // `GetLastError` (Windows) style error code. To check against common error
  // codes, reproc provides cross platform constants such as `REPROC_EPIPE` and
  // `REPROC_ETIMEDOUT`.
  if (r < 0) {
    goto finish;
  }

  // Close the stdin stream since we're not going to write any input to the
  // child process.
  r = reproc_close(process, REPROC_STREAM_IN);
  if (r < 0) {
    goto finish;
  }

  // Read the entire output of the child process. I've found this pattern to be
  // the most readable when reading the entire output of a child process. The
  // while loop keeps running until an error occurs in `reproc_read` (the child
  // process closing its output stream is also reported as an error).
  for (;;) {
    uint8_t buffer[4096];
    r = reproc_read(process, REPROC_STREAM_OUT, buffer, sizeof(buffer));
    if (r < 0) {
      break;
    }

    // On success, `reproc_read` returns the amount of bytes read.
    size_t bytes_read = (size_t) r;

    // Increase the size of `output` to make sure it can hold the new output.
    // This is definitely not the most performant way to grow a buffer so keep
    // that in mind. Add 1 to size to leave space for the NUL terminator which
    // isn't included in `output_size`.
    char *result = realloc(output, size + bytes_read + 1);
    if (result == NULL) {
      r = REPROC_ENOMEM;
      goto finish;
    }

    output = result;

    // Copy new data into `output`.
    memcpy(output + size, buffer, bytes_read);
    output[size + bytes_read] = '\0';
    size += bytes_read;
  }

  // Check that the while loop stopped because the output stream of the child
  // process was closed and not because of any other error.
  if (r != REPROC_EPIPE) {
    goto finish;
  }

  printf("%s", output);

  // Wait for the process to exit. This should always be done since some systems
  // (POSIX) don't clean up system resources allocated to a child process until
  // the parent process explicitly waits for it after it has exited.
  r = reproc_wait(process, REPROC_INFINITE);
  if (r < 0) {
    goto finish;
  }

finish:
  free(output);

  // Clean up all the resources allocated to the child process (including the
  // memory allocated by `reproc_new`). Unless custom stop actions are passed to
  // `reproc_start`, `reproc_destroy` will first wait indefinitely for the child
  // process to exit.
  reproc_destroy(process);

  if (r < 0) {
    fprintf(stderr, "%s\n", reproc_strerror(r));
  }

  return abs(r);
}
