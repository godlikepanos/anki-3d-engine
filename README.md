[![AnKi logo](http://anki3d.org/wp-content/uploads/2015/11/logo_248.png)](http://anki3d.org)

[![Build Status](https://travis-ci.org/godlikepanos/anki-3d-engine.svg?branch=master)](https://travis-ci.org/godlikepanos/anki-3d-engine)

AnKi 3D engine is a Linux and Windows opensource game engine that runs on
OpenGL 4.4.

License
=======

AnKi's license is BSD. This practicaly means that you can use the source or
parts of the source on proprietary and non proprietary products as long as you
follow the conditions of the license.

See LICENSE file for more info.

Building AnKi
=============

AnKi's build system is using CMake. A great effort was made to ease the building
process that's why the number of external dependencies are almost none.

On Linux
--------

Prerequisites:

- Cmake 2.8 and up
- GCC 4.8 and up or Clang 3.5 and up

To build the release version:

	$cd <path_to_anki>/build
	$cmake ..
	$make

To view and configure the build options you can use ccmake tool or similar:

	$cd <path_to_anki>/build
	$ccmake .

This will open an interface with all the available options.

On Windows
----------

Prerequisites:

- CMake 2.8 and up
- Mingw-w64 4.8 and up
	- Install to a path without spaces
	- Append the path where mingw's binaries are located (eg
	  C:/mingw-w64/x86_64-4.9.3-win32-seh-rt_v4-rev1/mingw64/bin) to the PATH
	  environment variable

To build the release version:

- Open CMake GUI tool
	- Point the source directory to where AnKi's CMakeLists.txt is located
	- Select a build directory
	- Configure by selecting mingw makefiles
	- Generate the makefiles
- Open a PowerShell
	- Navigate to where the build directory is located
	- Invoke the mingw's make: mingw32-make

> NOTE: If you have a better way to build on Windows please let us know.
