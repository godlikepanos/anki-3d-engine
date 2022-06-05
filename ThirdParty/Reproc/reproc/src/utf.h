#pragma once

#include <wchar.h>

// `size` represents the entire size of `string`, including NUL-terminators. We
// take the entire size because strings like the environment string passed to
// CreateProcessW includes multiple NUL-terminators so we can't always rely on
// `strlen` to calculate the string length for us. See the lpEnvironment
// documentation of CreateProcessW:
// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw
// Pass -1 as the size to have `utf16_from_utf8` calculate the size until (and
// including) the first NUL terminator.
wchar_t *utf16_from_utf8(const char *string, int size);
