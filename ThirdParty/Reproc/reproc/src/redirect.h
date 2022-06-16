#pragma once

#include <reproc/reproc.h>

#include "handle.h"
#include "pipe.h"

int redirect_init(pipe_type *parent,
                  handle_type *child,
                  REPROC_STREAM stream,
                  reproc_redirect redirect,
                  bool nonblocking,
                  handle_type out);

handle_type redirect_destroy(handle_type child, REPROC_REDIRECT type);

// Internal prototypes

int redirect_parent(handle_type *child, REPROC_STREAM stream);

int redirect_discard(handle_type *child, REPROC_STREAM stream);

int redirect_file(handle_type *child, FILE *file);

int redirect_path(handle_type *child, REPROC_STREAM stream, const char *path);
