#pragma once

#include <stdbool.h>
#include <stdio.h>

#if defined(_WIN32)
typedef void *handle_type; // `HANDLE`
#else
typedef int handle_type; // fd
#endif

extern const handle_type HANDLE_INVALID;

// Sets the `FD_CLOEXEC` flag on the file descriptor. POSIX only.
int handle_cloexec(handle_type handle, bool enable);

// Closes `handle` if it is not an invalid handle and returns an invalid handle.
// Does not overwrite the last system error if an error occurs while closing
// `handle`.
handle_type handle_destroy(handle_type handle);
