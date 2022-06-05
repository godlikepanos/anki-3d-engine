#include "redirect.h"

#include "error.h"

static int redirect_pipe(pipe_type *parent,
                         handle_type *child,
                         REPROC_STREAM stream,
                         bool nonblocking)
{
  ASSERT(parent);
  ASSERT(child);

  pipe_type pipe[] = { PIPE_INVALID, PIPE_INVALID };
  int r = -1;

  r = pipe_init(&pipe[0], &pipe[1]);
  if (r < 0) {
    goto finish;
  }

  r = pipe_nonblocking(stream == REPROC_STREAM_IN ? pipe[1] : pipe[0],
                       nonblocking);
  if (r < 0) {
    goto finish;
  }

  *parent = stream == REPROC_STREAM_IN ? pipe[1] : pipe[0];
  *child = stream == REPROC_STREAM_IN ? (handle_type) pipe[0]
                                      : (handle_type) pipe[1];

finish:
  if (r < 0) {
    pipe_destroy(pipe[0]);
    pipe_destroy(pipe[1]);
  }

  return r;
}

int redirect_init(pipe_type *parent,
                  handle_type *child,
                  REPROC_STREAM stream,
                  reproc_redirect redirect,
                  bool nonblocking,
                  handle_type out)
{
  ASSERT(parent);
  ASSERT(child);

  int r = REPROC_EINVAL;

  switch (redirect.type) {

    case REPROC_REDIRECT_DEFAULT:
      ASSERT(false);
      break;

    case REPROC_REDIRECT_PIPE:
      r = redirect_pipe(parent, child, stream, nonblocking);
      break;

    case REPROC_REDIRECT_PARENT:
      r = redirect_parent(child, stream);
      if (r == REPROC_EPIPE) {
        // Discard if the corresponding parent stream is closed.
        r = redirect_discard(child, stream);
      }

      if (r < 0) {
        break;
      }

      *parent = PIPE_INVALID;

      break;

    case REPROC_REDIRECT_DISCARD:
      r = redirect_discard(child, stream);
      if (r < 0) {
        break;
      }

      *parent = PIPE_INVALID;

      break;

    case REPROC_REDIRECT_HANDLE:
      ASSERT(redirect.handle);

      r = 0;

      *child = redirect.handle;
      *parent = PIPE_INVALID;

      break;

    case REPROC_REDIRECT_FILE:
      ASSERT(redirect.file);

      r = redirect_file(child, redirect.file);
      if (r < 0) {
        break;
      }

      *parent = PIPE_INVALID;

      break;

    case REPROC_REDIRECT_STDOUT:
      ASSERT(stream == REPROC_STREAM_ERR);
      ASSERT(out != HANDLE_INVALID);

      r = 0;

      *child = out;
      *parent = PIPE_INVALID;

      break;

    case REPROC_REDIRECT_PATH:
      ASSERT(redirect.path);

      r = redirect_path(child, stream, redirect.path);
      if (r < 0) {
        break;
      }

      *parent = PIPE_INVALID;

      break;
  }

  return r;
}

handle_type redirect_destroy(handle_type child, REPROC_REDIRECT type)
{
  if (child == HANDLE_INVALID) {
    return HANDLE_INVALID;
  }

  switch (type) {
    case REPROC_REDIRECT_DEFAULT:
      ASSERT(false);
      break;
    case REPROC_REDIRECT_PIPE:
      // We know `handle` is a pipe if `REDIRECT_PIPE` is used so the cast is
      // safe. This little hack prevents us from having to introduce a generic
      // handle type.
      pipe_destroy((pipe_type) child);
      break;
    case REPROC_REDIRECT_DISCARD:
    case REPROC_REDIRECT_PATH:
      handle_destroy(child);
      break;
    case REPROC_REDIRECT_PARENT:
    case REPROC_REDIRECT_FILE:
    case REPROC_REDIRECT_HANDLE:
    case REPROC_REDIRECT_STDOUT:
      break;
  }

  return HANDLE_INVALID;
}
