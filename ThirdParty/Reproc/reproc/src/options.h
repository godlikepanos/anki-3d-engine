#pragma once

#include <reproc/reproc.h>

reproc_stop_actions parse_stop_actions(reproc_stop_actions stop);

int parse_options(reproc_options *options, const char *const *argv);
