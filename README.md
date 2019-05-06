[![AnKi logo](http://anki3d.org/wp-content/uploads/2015/11/logo_248.png)](http://anki3d.org)

AnKi 3D engine is a Linux and Windows opensource game engine that runs on Vulkan 1.1 and OpenGL 4.5 (now deprecated).

[![Video](http://img.youtube.com/vi/va7nZ2EFR4c/0.jpg)](http://www.youtube.com/watch?v=va7nZ2EFR4c)

1 License
=========

AnKi's license is BSD. This practically means that you can use the source or parts of the source on proprietary and non
proprietary products as long as you follow the conditions of the license.

See `LICENSE` file for more info.

2 Building AnKi
===============

Build Status, Linux and Windows
[![Build Status](https://travis-ci.org/godlikepanos/anki-3d-engine.svg?branch=master)](https://travis-ci.org/godlikepanos/anki-3d-engine)

To checkout the source including the submodules type:

	git clone --recurse-submodules https://github.com/godlikepanos/anki-3d-engine.git anki

AnKi's build system is using `CMake`. A great effort was made to ease the building process that's why the number of
external dependencies are almost none.

2.1 On Linux
------------

Prerequisites:

- Cmake 3.0 and up
- GCC 5.0 and up or Clang 6.0 and up
- libx11-dev installed
- libxrandr-dev installed
- libx11-xcb-dev installed
- [Optional] libxinerama-dev if you want proper multi-monitor support

To build the release version:

	$cd path/to/anki
	$mkdir build
	$cd ./build
	$cmake .. -DCMAKE_BUILD_TYPE=Release
	$make

To view and configure the build options you can use `ccmake` tool or other similar tool:

	$cd path/to/anki/build
	$ccmake .

This will open an interface with all the available options.

2.2 On Windows
--------------

Prerequisites:

- Cmake 3.0 and up
- VulkanSDK version 1.1.x and up
	- Add an environment variable named `VULKAN_SDK` that points to the installation path of VulkanSDK
- Python 3.0 and up
	- Make sure that the python executable's location is in `PATH` environment variable
- Microsoft Visual Studio 2017 and up

To build the release version open `PowerShell` and type:

	$cd path/to/anki
	$mkdir build
	$cd build
	$cmake .. -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
	$cmake --build .

Alternatively, recent Visual Studio versions support building CMake projects from inside the IDE:

- Open Visual Studio
- Choose the "open folder" option and navigate to AnKi's checkout
- Visual Studio will automatically understand that AnKi is a CMake project and it will populate the CMake cache
- Press "build all"


3 Next steps
============

Try to build with `samples` enabled (search for the `ANKI_BUILD_SAMPLES=ON` option in your CMake GUI) and try running
the sponza executable. Then you will be able to see sponza running in AnKi. All samples must run from within their
directory.

	$cd path/to/anki/samples/sponza
	$./path/to/build/bin/sponza

More samples will follow.
