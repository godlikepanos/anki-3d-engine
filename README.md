[![AnKi logo](http://anki3d.org/wp-content/uploads/2015/11/logo_248.png)](http://anki3d.org)

[**Official website:** anki3d.org](https://www.anki3d.org)

[**Twitter development news feed:** twitter.com/anki3d](https://twitter.com/anki3d)

**AnKi 3D engine** is a Linux, Windows and Android opensource game engine that runs on Vulkan.

[![Video](http://img.youtube.com/vi/va7nZ2EFR4c/0.jpg)](http://www.youtube.com/watch?v=va7nZ2EFR4c)

1 License
=========

AnKi's license is `BSD`. This practically means that you can use the source or parts of the source on proprietary and
non proprietary products as long as you follow the conditions of the license.

See the [LICENSE](LICENSE) file for more info.

2 Building AnKi
===============

![Build Status Linux](https://github.com/godlikepanos/anki-3d-engine/actions/workflows/Linux.yml/badge.svg)
![Build Status Windows](https://github.com/godlikepanos/anki-3d-engine/actions/workflows/Windows.yml/badge.svg)
![Build Status Android](https://github.com/godlikepanos/anki-3d-engine/actions/workflows/Android.yml/badge.svg)

To checkout the source type:

	git clone https://github.com/godlikepanos/anki-3d-engine.git anki

AnKi's build system is using `CMake`. A great effort was made to ease the building process that's why the number of
external dependencies are almost none.

2.1 On Linux
------------

Prerequisites:

- Cmake 3.10 and up
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

- Cmake 3.10 and up
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

2.3 For Android
---------------

Prerequisites:

- Android Studio
- From Android Studio's package manager you need to install `NDK` and `CMake`

Android builds work a bit differently from Linux and Windows. You are required to generate a gradle project per build
target. For example, if you want to generate a project for the `Sponza` sample just type from a Linux terminal:

	$cd path/to/anki
	$./Samples/Sponza/GenerateAndroidProject.sh

or from a PowerShell terminal on Windows:

	$cd path/to/anki
	$./Samples/Sponza/GenerateAndroidProject.bat

The `GenerateAndroidProject` scripts will generate a project in the root directory of AnKi. So for the `Sponza` sample
the script will create a directory named `AndroidProject_Sponza`.

Then you can open the `AndroidProject_Sponza` project from `Android Studio` and build it, debug it, run it etc.

3 Next steps
============

This code repository contains **4 sample projects** that are built by default (`ANKI_BUILD_SAMPLES` CMake option):

- `Sponza`: The Crytek's Sponza scene
- `SimpleScene`: A simple scene (Cornell box)
- `PhysicsPlayground`: A scene with programmer's art and some physics interactions
- `SkeletalAnimation`: A simple scene with an animated skin

You can try running them and interacting with them. To run sponza, for example, execute the binary from any working
directory.

On Linux:

	$./path/to/build/Bin/Sponza

On Windows just find the `Sponza.exe` and execute it. It's preferable to run the samples from a terminal because that
prints some information, including possible errors.

4 Contributing
==============

There are no special rules if you want to contribute. Just create a PR. But before you do that, carefully read the
[code style guide](Docs/CodeStyle.md).
