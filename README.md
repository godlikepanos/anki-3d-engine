[![AnKi logo](http://anki3d.org/wp-content/uploads/2015/11/logo_248.png)](http://anki3d.org)

[![Build Status](https://travis-ci.org/godlikepanos/anki-3d-engine.svg?branch=master)](https://travis-ci.org/godlikepanos/anki-3d-engine)

AnKi 3D engine is a Linux and Windows opensource game engine that runs on
OpenGL 4.5 and Vulkan 1.0 (Beta).

[![Video](http://img.youtube.com/vi/va7nZ2EFR4c/0.jpg)](http://www.youtube.com/watch?v=va7nZ2EFR4c)

License
=======

AnKi's license is BSD. This practically means that you can use the source or parts of the source on proprietary and non 
proprietary products as long as you follow the conditions of the license.

See LICENSE file for more info.

Building AnKi
=============

To checkout the source including the submodules type:

	git clone --recurse-submodules https://github.com/godlikepanos/anki-3d-engine.git anki

AnKi's build system is using CMake. A great effort was made to ease the building process that's why the number of 
external dependencies are almost none.

On Linux
--------

Prerequisites:

- Cmake 2.8 and up
- GCC 4.8 and up or Clang 3.7 and up
- libx11-dev installed
- [Optional] libxinerama-dev if you want proper multi-monitor support

To build the release version:

	$cd path/to/anki
	$cd mkdir build
	$cd ./build
	$cmake .. -DCMAKE_BUILD_TYPE=Release
	$make

To view and configure the build options you can use ccmake tool or other similar tool:

	$cd <path_to_anki>/build
	$ccmake .

This will open an interface with all the available options.

On Windows
----------

Prerequisites:

- CMake 2.8 and up
- Mingw-w64 4.8 and up
	- Install to a path without spaces
	- Append the path where mingw's binaries are located (eg C:/mingw-w64/x86_64-4.9.3-win32-seh-rt_v4-rev1/mingw64/bin)
	  to the PATH environment variable

To build the release version:

- Open CMake GUI tool
	- Point the source directory to where AnKi's CMakeLists.txt is located
	- Select a build directory (eg path/to/anki/build)
	- Configure by selecting mingw makefiles
	- Generate the makefiles
- Open a PowerShell
	- Navigate to where the build directory is located
	- Invoke the mingw's make: mingw32-make

> NOTE: If you have a better way to build on Windows please let us know.

> NOTE 2: The Windows build tends to break often since Windows is not the primary development platform. Please report 
> any bugs.

Next steps
==========

Try to build with samples enabled (see the relevant option in your CMake GUI, ANKI_BUILD_SAMPLES) and try running the 
sponza executable. Then you will be able to see sponza running in AnKi. All samples must run from within their 
directory.

	$cd path/to/anki/samples/sponza
	$./path/to/build/samples/sponza/sponza

More samples will follow.
