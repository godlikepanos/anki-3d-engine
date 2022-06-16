#include <reproc/drain.h>
#include <reproc/reproc.h>

#include "assert.h"

#define MESSAGE "reproc stands for REdirected PROCess"

static void io()
{
  int r = -1;

  reproc_t *process = reproc_new();
  ASSERT(process);

  const char *argv[] = { RESOURCE_DIRECTORY "/io", NULL };

  r = reproc_start(process, argv,
                   (reproc_options){
                       .redirect.err.type = REPROC_REDIRECT_STDOUT });
  ASSERT_OK(r);

  r = reproc_write(process, (uint8_t *) MESSAGE, strlen(MESSAGE));
  ASSERT_OK(r);
  ASSERT_EQ_INT(r, (int) strlen(MESSAGE));

  r = reproc_close(process, REPROC_STREAM_IN);
  ASSERT_OK(r);

  char *out = NULL;
  r = reproc_drain(process, reproc_sink_string(&out), REPROC_SINK_NULL);
  ASSERT_OK(r);

  ASSERT(out != NULL);
  ASSERT_EQ_STR(out, MESSAGE MESSAGE);

  r = reproc_wait(process, REPROC_INFINITE);
  ASSERT_OK(r);

  reproc_destroy(process);

  reproc_free(out);
}

static void timeout(void)
{
  int r = -1;

  reproc_t *process = reproc_new();
  ASSERT(process);

  const char *argv[] = { RESOURCE_DIRECTORY "/io", NULL };

  r = reproc_start(process, argv, (reproc_options){ 0 });
  ASSERT_OK(r);

  reproc_event_source source = { process, REPROC_EVENT_OUT | REPROC_EVENT_ERR,
                                 0 };
  r = reproc_poll(&source, 1, 200);
  ASSERT(r == 0);

  r = reproc_close(process, REPROC_STREAM_IN);
  ASSERT_OK(r);

  r = reproc_poll(&source, 1, 200);
  ASSERT_OK(r);

  reproc_destroy(process);
}

int main(void)
{
  io();
  timeout();
}
