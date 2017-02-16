[![AnKi logo](http://anki3d.org/wp-content/uploads/2015/11/logo_248.png)](http://anki3d.org)

AnKi 3D engine is a Linux and Windows opensource game engine that runs on
OpenGL 4.5 and Vulkan 1.0 (Experimental).

[![Video](http://img.youtube.com/vi/va7nZ2EFR4c/0.jpg)](http://www.youtube.com/watch?v=va7nZ2EFR4c)

1) License
==========

AnKi's license is BSD. This practically means that you can use the source or parts of the source on proprietary and non 
proprietary products as long as you follow the conditions of the license.

See LICENSE file for more info.

2) Building AnKi
================

| OS      | Master Branch Build Status                                                                                                                                    |
| ------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Linux   | [![Build Status Linux](https://travis-ci.org/godlikepanos/anki-3d-engine.svg?branch=master)](https://travis-ci.org/godlikepanos/anki-3d-engine)               | 
| Windows | [![Build status Windows](https://ci.appveyor.com/api/projects/status/waij29m7o8ajjoqh?svg=true)](https://ci.appveyor.com/project/godlikepanos/anki-3d-engine) |

To checkout the source including the submodules type:

	git clone --recurse-submodules https://github.com/godlikepanos/anki-3d-engine.git anki

AnKi's build system is using CMake. A great effort was made to ease the building process that's why the number of 
external dependencies are almost none.

2.1) On Linux
-------------

Prerequisites:

- Cmake 2.8 and up
- GCC 4.8 and up or Clang 3.7 and up
- libx11-dev installed
- [Optional] libxinerama-dev if you want proper multi-monitor support

To build the release version:

	$cd path/to/anki
	$mkdir build
	$cd ./build
	$cmake .. -DCMAKE_BUILD_TYPE=Release
	$make

To view and configure the build options you can use ccmake tool or other similar tool:

	$cd path/to/anki/build
	$ccmake .

This will open an interface with all the available options.

2.2) On Windows
---------------

Prerequisites:

- CMake 2.8 and up
	- Make sure you add cmake.exe to your PATH environment variable (The installer asks, press yes)
- MinGW-w64 4.8 and up
	- MinGW has many variants. You need the POSIX version plus SEH (eg x86_64-posix-seh)
	- Install to a path without spaces (eg C:/mingw-w64)
	- Append the path where mingw's binaries are located (eg C:/mingw-w64/bin) to the PATH environment variable

To build the release version open PowerShell and type:

	$cd path/to/anki
	$mkdir build
	$cd build
	$cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
	$mingw32-make

> NOTE: If you have a better way to build on Windows please let us know.

3) Next steps
=============

Try to build with samples enabled (search for the ANKI_BUILD_SAMPLES option in your CMake GUI) and try running the 
sponza executable. Then you will be able to see sponza running in AnKi. All samples must run from within their 
directory.

	$cd path/to/anki/samples/sponza
	$./path/to/build/samples/sponza/sponza

More samples will follow.
