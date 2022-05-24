#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ${0} <path to ShaderCompiler>"
	exit 0
fi

./Tools/Android/GenerateAndroidProject.py -o . -t Tests -a ./Tests/Assets/ --shader-compiler $1
