#define _POSIX_C_SOURCE 200809L

#include "pipe.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "handle.h"

const int PIPE_INVALID = -1;

const short PIPE_EVENT_IN = POLLIN;
const short PIPE_EVENT_OUT = POLLOUT;

int pipe_init(int *read, int *write)
{
  ASSERT(read);
  ASSERT(write);

  int pair[] = { PIPE_INVALID, PIPE_INVALID };
  int r = -1;

  r = pipe(pair);
  if (r < 0) {
    r = -errno;
    goto finish;
  }

  r = handle_cloexec(pair[0], true);
  if (r < 0) {
    goto finish;
  }

  r = handle_cloexec(pair[1], true);
  if (r < 0) {
    goto finish;
  }

  *read = pair[0];
  *write = pair[1];

  pair[0] = PIPE_INVALID;
  pair[1] = PIPE_INVALID;

finish:
  pipe_destroy(pair[0]);
  pipe_destroy(pair[1]);

  return r;
}

int pipe_nonblocking(int pipe, bool enable)
{
  int r = -1;

  r = fcntl(pipe, F_GETFL, 0);
  if (r < 0) {
    return -errno;
  }

  r = enable ? r | O_NONBLOCK : r & ~O_NONBLOCK;

  r = fcntl(pipe, F_SETFL, r);

  return r < 0 ? -errno : 0;
}

int pipe_read(int pipe, uint8_t *buffer, size_t size)
{
  ASSERT(pipe != PIPE_INVALID);
  ASSERT(buffer);

  int r = (int) read(pipe, buffer, size);

  if (r == 0) {
    // `read` returns 0 to indicate the other end of the pipe was closed.
    return -EPIPE;
  }

  return r < 0 ? -errno : r;
}

int pipe_write(int pipe, const uint8_t *buffer, size_t size)
{
  ASSERT(pipe != PIPE_INVALID);
  ASSERT(buffer);

  int r = (int) write(pipe, buffer, size);

  return r < 0 ? -errno : r;
}

int pipe_poll(pipe_event_source *sources, size_t num_sources, int timeout)
{
  ASSERT(num_sources <= INT_MAX);

  struct pollfd *pollfds = NULL;
  int r = -1;

  pollfds = calloc(num_sources, sizeof(struct pollfd));
  if (pollfds == NULL) {
    r = -errno;
    goto finish;
  }

  for (size_t i = 0; i < num_sources; i++) {
    pollfds[i].fd = sources[i].pipe;
    pollfds[i].events = sources[i].interests;
  }

  r = poll(pollfds, (nfds_t) num_sources, timeout);
  if (r < 0) {
    r = -errno;
    goto finish;
  }

  for (size_t i = 0; i < num_sources; i++) {
    sources[i].events = pollfds[i].revents;
  }

finish:
  free(pollfds);

  return r;
}

int pipe_shutdown(int pipe)
{
  (void) pipe;
  return 0;
}

int pipe_destroy(int pipe)
{
  return handle_destroy(pipe);
}
