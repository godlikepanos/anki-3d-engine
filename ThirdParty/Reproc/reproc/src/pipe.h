#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN64
typedef uint64_t pipe_type; // `SOCKET`
#elif _WIN32
typedef uint32_t pipe_type; // `SOCKET`
#else
typedef int pipe_type; // fd
#endif

extern const pipe_type PIPE_INVALID;

extern const short PIPE_EVENT_IN;
extern const short PIPE_EVENT_OUT;

typedef struct {
  pipe_type pipe;
  short interests;
  short events;
} pipe_event_source;

// Creates a new anonymous pipe. `parent` and `child` are set to the parent and
// child endpoint of the pipe respectively.
int pipe_init(pipe_type *read, pipe_type *write);

// Sets `pipe` to nonblocking mode.
int pipe_nonblocking(pipe_type pipe, bool enable);

// Reads up to `size` bytes into `buffer` from the pipe indicated by `pipe` and
// returns the amount of bytes read.
int pipe_read(pipe_type pipe, uint8_t *buffer, size_t size);

// Writes up to `size` bytes from `buffer` to the pipe indicated by `pipe` and
// returns the amount of bytes written.
int pipe_write(pipe_type pipe, const uint8_t *buffer, size_t size);

// Polls the given event sources for events.
int pipe_poll(pipe_event_source *sources, size_t num_sources, int timeout);

int pipe_shutdown(pipe_type pipe);

pipe_type pipe_destroy(pipe_type pipe);
