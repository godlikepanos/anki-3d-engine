#pragma once

#define STRV_FOREACH(s, l) for ((s) = (l); (s) && *(s); (s)++)

char **strv_concat(char *const *a, const char *const *b);

char **strv_free(char **l);
