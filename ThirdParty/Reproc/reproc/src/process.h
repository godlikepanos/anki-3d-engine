#pragma once

#include "handle.h"

#include <stdbool.h>

#include <reproc/reproc.h>

#if defined(_WIN32)
typedef void *process_type; // `HANDLE`
#else
typedef int process_type; // `pid_t`
#endif

extern const process_type PROCESS_INVALID;

struct process_options {
  // If `NULL`, the child process inherits the environment of the current
  // process.
  struct {
    REPROC_ENV behavior;
    const char *const *extra;
  } env;
  // If not `NULL`, the working directory of the child process is set to
  // `working_directory`.
  const char *working_directory;
  // The standard streams of the child process are redirected to the `in`, `out`
  // and `err` handles. If a handle is `HANDLE_INVALID`, the corresponding child
  // process standard stream is closed. The `exit` handle is simply inherited by
  // the child process.
  struct {
    handle_type in;
    handle_type out;
    handle_type err;
    handle_type exit;
  } handle;
};

// Spawns a child process that executes the command stored in `argv`.
//
// If `argv` is `NULL` on POSIX, `exec` is not called after fork and this
// function returns 0 in the child process and > 0 in the parent process. On
// Windows, if `argv` is `NULL`, an error is returned.
//
// The process handle of the new child process is assigned to `process`.
int process_start(process_type *process,
                  const char *const *argv,
                  struct process_options options);

// Returns the process ID associated with the given handle. On posix systems the
// handle is the process ID and so its returned directly. On WIN32 the process
// ID is returned from GetProcessId on the pointer.
int process_pid(process_type process);

// Returns the process's exit status if it has finished running.
int process_wait(process_type process);

// Sends the `SIGTERM` (POSIX) or `CTRL-BREAK` (Windows) signal to the process
// indicated by `process`.
int process_terminate(process_type process);

// Sends the `SIGKILL` signal to `process` (POSIX) or calls `TerminateProcess`
// on `process` (Windows).
int process_kill(process_type process);

process_type process_destroy(process_type process);
