#!/bin/bash
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")"

xmls="Scene.xml Math.xml Renderer.xml Logger.xml"

for xml in $xmls; do
	python3 LuaGlueGen.py -i "$xml"
done

python3 LuaDefsGen.py -i "$(echo $xmls | tr ' ' ':')" -o AnKiScriptApi.lua
