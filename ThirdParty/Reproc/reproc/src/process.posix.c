#define _POSIX_C_SOURCE 200809L

#include "process.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error.h"
#include "macro.h"
#include "pipe.h"
#include "strv.h"

#define CWD_BUF_SIZE_INCREMENT 4096

const pid_t PROCESS_INVALID = -1;

static int signal_mask(int how, const sigset_t *newmask, sigset_t *oldmask)
{
  int r = -1;

#if defined(REPROC_MULTITHREADED)
  // `pthread_sigmask` returns positive errno values so we negate them.
  r = -pthread_sigmask(how, newmask, oldmask);
#else
  r = sigprocmask(how, newmask, oldmask);
  r = r < 0 ? -errno : 0;
#endif

  return r;
}

// Returns true if the NUL-terminated string indicated by `path` is a relative
// path. A path is relative if any character except the first is a forward slash
// ('/').
static bool path_is_relative(const char *path)
{
  return strlen(path) > 0 && path[0] != '/' && strchr(path + 1, '/') != NULL;
}

// Prepends the NUL-terminated string indicated by `path` with the current
// working directory. The caller is responsible for freeing the result of this
// function. If an error occurs, `NULL` is returned and `errno` is set to
// indicate the error.
static char *path_prepend_cwd(const char *path)
{
  ASSERT(path);

  size_t path_size = strlen(path);
  size_t cwd_size = CWD_BUF_SIZE_INCREMENT;

  // We always allocate sufficient space for `path` but do not include this
  // space in `cwd_size` so we can be sure that when `getcwd` succeeds there is
  // sufficient space left in `cwd` to append `path`.

  // +2 reserves space to add a NUL terminator and potentially a missing '/'
  // after the current working directory.
  char *cwd = calloc(cwd_size + path_size + 2, sizeof(char));
  if (cwd == NULL) {
    return cwd;
  }

  while (getcwd(cwd, cwd_size) == NULL) {
    if (errno != ERANGE) {
      free(cwd);
      return NULL;
    }

    cwd_size += CWD_BUF_SIZE_INCREMENT;

    char *result = realloc(cwd, cwd_size + path_size + 1);
    if (result == NULL) {
      free(cwd);
      return result;
    }

    cwd = result;
  }

  cwd_size = strlen(cwd);

  // Add a forward slash after `cwd` if there is none.
  if (cwd[cwd_size - 1] != '/') {
    cwd[cwd_size] = '/';
    cwd[cwd_size + 1] = '\0';
    cwd_size++;
  }

  // We've made sure there's sufficient space left in `cwd` to add `path` and a
  // NUL terminator.
  memcpy(cwd + cwd_size, path, path_size);
  cwd[cwd_size + path_size] = '\0';

  return cwd;
}

static const int MAX_FD_LIMIT = 1024 * 1024;

static int get_max_fd(void)
{
  struct rlimit limit = { 0 };

  int r = getrlimit(RLIMIT_NOFILE, &limit);
  if (r < 0) {
    return -errno;
  }

  rlim_t soft = limit.rlim_cur;

  if (soft == RLIM_INFINITY || soft > INT_MAX) {
    return INT_MAX;
  }

  return (int) (soft - 1);
}

static bool fd_in_set(int fd, const int *fd_set, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    if (fd == fd_set[i]) {
      return true;
    }
  }

  return false;
}

static pid_t process_fork(const int *except, size_t num_except)
{
  struct {
    sigset_t old;
    sigset_t new;
  } mask;

  int r = -1;

  // We don't want signal handlers of the parent to run in the child process so
  // we block all signals before forking.

  r = sigfillset(&mask.new);
  if (r < 0) {
    return -errno;
  }

  r = signal_mask(SIG_SETMASK, &mask.new, &mask.old);
  if (r < 0) {
    return r;
  }

  struct {
    int read;
    int write;
  } pipe = { PIPE_INVALID, PIPE_INVALID };

  r = pipe_init(&pipe.read, &pipe.write);
  if (r < 0) {
    return r;
  }

  r = fork();
  if (r < 0) {
    // `fork` error.

    r = -errno; // Save `errno`.

    int q = signal_mask(SIG_SETMASK, &mask.new, &mask.old);
    ASSERT_UNUSED(q == 0);

    pipe_destroy(pipe.read);
    pipe_destroy(pipe.write);

    return r;
  }

  if (r > 0) {
    // Parent process

    pid_t child = r;

    // From now on, the child process might have started so we don't report
    // errors from `signal_mask` and `read`. This puts the responsibility
    // for cleaning up the process in the hands of the caller.

    int q = signal_mask(SIG_SETMASK, &mask.old, &mask.old);
    ASSERT_UNUSED(q == 0);

    // Close the error pipe write end on the parent's side so `read` will return
    // when it is closed on the child side as well.
    pipe_destroy(pipe.write);

    int child_errno = 0;
    q = (int) read(pipe.read, &child_errno, sizeof(child_errno));
    ASSERT_UNUSED(q >= 0);

    if (child_errno > 0) {
      // If the child writes to the error pipe and exits, we're certain the
      // child process exited on its own and we can report errors as usual.
      r = waitpid(child, NULL, 0);
      ASSERT(r < 0 || r == child);

      r = r < 0 ? -errno : -child_errno;
    }

    pipe_destroy(pipe.read);

    return r < 0 ? r : child;
  }

  // Child process

  // Reset all signal handlers so they don't run in the child process. By
  // default, a child process inherits the parent's signal handlers but we
  // override this as most signal handlers won't be written in a way that they
  // can deal with being run in a child process.

  struct sigaction action = { .sa_handler = SIG_DFL };

  r = sigemptyset(&action.sa_mask);
  if (r < 0) {
    r = -errno;
    goto finish;
  }

  // NSIG is not standardized so we use a fixed limit instead.
  for (int signal = 0; signal < 32; signal++) {
    r = sigaction(signal, &action, NULL);
    if (r < 0 && errno != EINVAL) {
      r = -errno;
      goto finish;
    }
  }

  // Reset the child's signal mask to the default signal mask. By default, a
  // child process inherits the parent's signal mask (even over an `exec` call)
  // but we override this as most processes won't be written in a way that they
  // can deal with starting with a custom signal mask.

  r = sigemptyset(&mask.new);
  if (r < 0) {
    r = -errno;
    goto finish;
  }

  r = signal_mask(SIG_SETMASK, &mask.new, NULL);
  if (r < 0) {
    goto finish;
  }

  // Not all file descriptors might have been created with the `FD_CLOEXEC`
  // flag so we manually close all file descriptors to prevent file descriptors
  // leaking into the child process.

  r = get_max_fd();
  if (r < 0) {
    goto finish;
  }

  int max_fd = r;

  if (max_fd > MAX_FD_LIMIT) {
    // Refuse to try to close too many file descriptors.
    r = -EMFILE;
    goto finish;
  }

  for (int i = 0; i < max_fd; i++) {
    // Make sure we don't close the error pipe file descriptors twice.
    if (i == pipe.read || i == pipe.write) {
      continue;
    }

    if (fd_in_set(i, except, num_except)) {
      continue;
    }

    // Check if `i` is a valid file descriptor before trying to close it.
    r = fcntl(i, F_GETFD);
    if (r >= 0) {
      handle_destroy(i);
    }
  }

  r = 0;

finish:
  if (r < 0) {
    (void) !write(pipe.write, &errno, sizeof(errno));
    _exit(EXIT_FAILURE);
  }

  pipe_destroy(pipe.write);
  pipe_destroy(pipe.read);

  return 0;
}

int process_start(pid_t *process,
                  const char *const *argv,
                  struct process_options options)
{
  ASSERT(process);

  if (argv != NULL) {
    ASSERT(argv[0] != NULL);
  }

  struct {
    int read;
    int write;
  } pipe = { PIPE_INVALID, PIPE_INVALID };
  char *program = NULL;
  char **env = NULL;
  int r = -1;

  // We create an error pipe to receive errors from the child process.
  r = pipe_init(&pipe.read, &pipe.write);
  if (r < 0) {
    goto finish;
  }

  if (argv != NULL) {
    // We prepend the parent working directory to `program` if it is a
    // relative path so that it will always be searched for relative to the
    // parent working directory even after executing `chdir`.
    program = options.working_directory && path_is_relative(argv[0])
                  ? path_prepend_cwd(argv[0])
                  : strdup(argv[0]);
    if (program == NULL) {
      r = -errno;
      goto finish;
    }
  }

  extern char **environ; // NOLINT
  char *const *parent = options.env.behavior == REPROC_ENV_EMPTY ? NULL
                                                                 : environ;
  env = strv_concat(parent, options.env.extra);
  if (env == NULL) {
    goto finish;
  }

  int except[] = { options.handle.in, options.handle.out, options.handle.err,
                   pipe.read,         pipe.write,         options.handle.exit };

  r = process_fork(except, ARRAY_SIZE(except));
  if (r < 0) {
    goto finish;
  }

  if (r == 0) {
    // Redirect stdin, stdout and stderr.

    int redirect[] = { options.handle.in, options.handle.out,
                       options.handle.err };

    for (int i = 0; i < (int) ARRAY_SIZE(redirect); i++) {
      // `i` corresponds to the standard stream we need to redirect.
      r = dup2(redirect[i], i);
      if (r < 0) {
        r = -errno;
        goto child;
      }

      // Make sure we don't accidentally cloexec the standard streams of the
      // child process when we're inheriting the parent standard streams. If we
      // don't call `exec`, the caller is responsible for closing the redirect
      // and exit handles.
      if (redirect[i] != i) {
        // Make sure the pipe is closed when we call exec.
        r = handle_cloexec(redirect[i], true);
        if (r < 0) {
          goto child;
        }
      }
    }

    // Make sure the `exit` file descriptor is inherited.

    r = handle_cloexec(options.handle.exit, false);
    if (r < 0) {
      goto child;
    }

    if (options.working_directory != NULL) {
      r = chdir(options.working_directory);
      if (r < 0) {
        r = -errno;
        goto child;
      }
    }

    // `environ` is carried over calls to `exec`.
    environ = env;

    if (argv != NULL) {
      ASSERT(program);

      r = execvp(program, (char *const *) argv);
      if (r < 0) {
        r = -errno;
        goto child;
      }
    }

    env = NULL;

  child:
    if (r < 0) {
      (void) !write(pipe.write, &errno, sizeof(errno));
      _exit(EXIT_FAILURE);
    }

    pipe_destroy(pipe.read);
    pipe_destroy(pipe.write);
    free(program);
    strv_free(env);

    return 0;
  }

  pid_t child = r;

  // Close the error pipe write end on the parent's side so `read` will return
  // when it is closed on the child side as well.
  pipe.write = pipe_destroy(pipe.write);

  int child_errno = 0;
  r = (int) read(pipe.read, &child_errno, sizeof(child_errno));
  ASSERT_UNUSED(r >= 0);

  if (child_errno > 0) {
    r = waitpid(child, NULL, 0);
    r = r < 0 ? -errno : -child_errno;
    goto finish;
  }

  *process = child;
  r = 0;

finish:
  pipe_destroy(pipe.read);
  pipe_destroy(pipe.write);
  free(program);
  strv_free(env);

  return r < 0 ? r : 1;
}

static int parse_status(int status)
{
  return WIFEXITED(status) ? WEXITSTATUS(status) : WTERMSIG(status) + 128;
}

int process_pid(process_type process)
{
  return process;
}

int process_wait(pid_t process)
{
  ASSERT(process != PROCESS_INVALID);

  int status = 0;
  int r = waitpid(process, &status, 0);
  if (r < 0) {
    return -errno;
  }

  ASSERT(r == process);

  return parse_status(status);
}

int process_terminate(pid_t process)
{
  ASSERT(process != PROCESS_INVALID);

  int r = kill(process, SIGTERM);
  return r < 0 ? -errno : 0;
}

int process_kill(pid_t process)
{
  ASSERT(process != PROCESS_INVALID);

  int r = kill(process, SIGKILL);
  return r < 0 ? -errno : 0;
}

pid_t process_destroy(pid_t process)
{
  // `waitpid` already cleans up the process for us.
  (void) process;
  return PROCESS_INVALID;
}
