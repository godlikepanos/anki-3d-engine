#include "strv.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

static char *str_dup(const char *s)
{
  ASSERT_RETURN(s, NULL);

  char *r = malloc(strlen(s) + 1);
  if (!r) {
    return NULL;
  }

  strcpy(r, s); // NOLINT

  return r;
}

char **strv_concat(char *const *a, const char *const *b)
{
  char *const *i = NULL;
  const char *const *j = NULL;
  size_t size = 1;
  size_t c = 0;

  STRV_FOREACH(i, a) {
    size++;
  }

  STRV_FOREACH(j, b) {
    size++;
  }

  char **r = calloc(size, sizeof(char *));
  if (!r) {
    goto finish;
  }

  STRV_FOREACH(i, a) {
    r[c] = str_dup(*i);
    if (!r[c]) {
      goto finish;
    }

    c++;
  }

  STRV_FOREACH(j, b) {
    r[c] = str_dup(*j);
    if (!r[c]) {
      goto finish;
    }

    c++;
  }

  r[c++] = NULL;

finish:
  if (c < size) {
    STRV_FOREACH(i, r) {
      free(*i);
    }

    free(r);

    return NULL;
  }

  return r;
}

char **strv_free(char **l)
{
  char **s = NULL;

  STRV_FOREACH(s, l) {
    free(*s);
  }

  free(l);

  return NULL;
}
