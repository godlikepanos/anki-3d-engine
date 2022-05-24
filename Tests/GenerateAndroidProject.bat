@echo off
if "%~1"=="" (
	echo "Usage: %0 <path to ShaderCompiler.exe>"
	exit 0
)

python ./Tools/Android/GenerateAndroidProject.py -o . -t Tests -a ./Tests/Assets/ --shader-compiler %1
