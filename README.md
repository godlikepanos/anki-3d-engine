[![AnKi logo](http://anki3d.org/wp-content/uploads/2015/11/logo_248.png)](http://anki3d.org)

AnKi 3D engine is a Linux and Windows opensource game engine that runs on Vulkan 1.1 and OpenGL 4.5 (now deprecated).

[![Video](http://img.youtube.com/vi/va7nZ2EFR4c/0.jpg)](http://www.youtube.com/watch?v=va7nZ2EFR4c)

1 License
=========

AnKi's license is `BSD`. This practically means that you can use the source or parts of the source on proprietary and
non proprietary products as long as you follow the conditions of the license.

See the [LICENSE](LICENSE) file for more info.

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
	- Make sure that `Windows 10 SDK (xxx) for Desktop C++ [x86 and x64]` component is installed

To build the release version open `PowerShell` and type:

	$cd path/to/anki
	$mkdir build
	$cd build
	$cmake .. -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
	$cmake --build . --config Release

Alternatively, recent Visual Studio versions support building CMake projects from inside the IDE:

- Open Visual Studio
- Choose the "open a local folder" option and open AnKi's root directory (where this README.md is located)
- Visual Studio will automatically understand that AnKi is a CMake project and it will populate the CMake cache
- Press "build all"

3 Next steps
============

This code repository contains **4 sample projects** that are built by default (`ANKI_BUILD_SAMPLES` CMake option):

- `sponza`: The Crytek's Sponza scene
- `simple_scene`: A simple scene
- `physics_playground`: A scene with programmer's art and some physics interactions
- `skeletal_animation`: A simple scene with an animated skin

You can try running them and interacting with them. To run sponza, for example, execute the binary from any working
directory.

On Linux:

	$./path/to/build/bin/sponza

On Windows just find the `sponza.exe` and execute it. It's preferable to run the samples from a terminal because that
prints some information, including possible errors.

4 Contributing
==============

There are no special rules if you want to contribute. Just create a PR. Read the code [style guide](docs/code_style.md)
before that though.
