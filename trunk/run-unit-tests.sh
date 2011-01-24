#!/bin/bash

export LD_LIBRARY_PATH=extern/lib-x86-64-linux/
ulimit -c unlimited
unit-tests/build/anki-unit-tests $1
