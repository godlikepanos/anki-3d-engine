#include "utf.h"

#include <limits.h>
#include <stdlib.h>
#include <windows.h>

#include "error.h"

wchar_t *utf16_from_utf8(const char *string, int size)
{
  ASSERT(string);

  // Determine wstring size (`MultiByteToWideChar` returns the required size if
  // its last two arguments are `NULL` and 0).
  int r = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string, size, NULL,
                              0);
  if (r == 0) {
    return NULL;
  }

  // `MultiByteToWideChar` does not return negative values so the cast to
  // `size_t` is safe.
  wchar_t *wstring = calloc((size_t) r, sizeof(wchar_t));
  if (wstring == NULL) {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return NULL;
  }

  // Now we pass our allocated string and its size as the last two arguments
  // instead of `NULL` and 0 which makes `MultiByteToWideChar` actually perform
  // the conversion.
  r = MultiByteToWideChar(CP_UTF8, 0, string, size, wstring, r);
  if (r == 0) {
    free(wstring);
    return NULL;
  }

  return wstring;
}
