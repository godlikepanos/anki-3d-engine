#!/bin/bash
find anki/ tests/ tools/ -name '*.h' -or -name '*.cpp' -or -name '*.glsl' | while read file; do expand -t4 $file | cut -c81- | grep -qv '^$' && echo $file ; done
