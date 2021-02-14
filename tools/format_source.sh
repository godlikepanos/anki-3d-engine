#!/bin/bash

files=(`find ./anki ./Tests ./Sandbox ./tools ./samples -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cpp' -o -name '*.glsl' -o -name '*.glslp' -o -name '*.ankiprog'`)

filecount=${#files[@]}

count=0
for f in ${files[@]}
do
	# Run it in parallel
	echo -ne Formatting ${count}/${filecount}\\r
	./ThirdParty/Bin/clang-format -sort-includes=false -i ${f} &
	count=$((${count}+1))

	# Throttle the parallel commands
	if !((count % 16)); then
		wait
	fi
done

wait

echo Done! Formatted ${filecount} files


