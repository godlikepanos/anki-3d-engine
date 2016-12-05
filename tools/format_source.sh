#!/bin/bash

files=(`find ./src ./tests ./sandbox ./tools ./shaders ./samples -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cpp' -o -name '*.glsl'`)

filecount=${#files[@]}

count=0
for f in ${files[@]}
do
	echo -ne Formatting ${count}/${filecount}\\r
	./thirdparty/bin/clang-format -sort-includes=false -i ${f}
	count=$((${count}+1))
done

echo Done! Formatted ${filecount} files


