#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ${0} <path to ShaderCompiler>"
	exit 0
fi

./Tools/Android/GenerateAndroidProject.py -o . -t Sandbox --shader-compiler $1
