AnKi 3D engine is a Linux opensource game engine build using OpenGL 4.4.

## License

AnKi's license is BSD. This practicaly means that you can use the source or
parts of the source on proprietary and non proprietary products as long as you
follow the conditions of the license.

See LICENSE file for more info.

## Building AnKi

AnKi's build system is using CMake. A great effort was made to keep the number 
of external dependencies to minimum so the only prerequisites are the following:

- X11 development files (Package name under Ubuntu: libx11-dev)
- Mesa GL development files (Package name under Ubuntu: mesa-common-dev)

AnKi is using the C++11 standard so the supported compilers are:

- GCC 4.8 or greater 
- clang 3.3 or greater 

To build the release version:

	$cd <path_to_anki>/build
	$cmake -DANKI_BUILD_TYPE=Release ..
	$make

To view and configure the build options you can use ccmake tool or similar app:

	$cd <path_to_anki>/build
	$ccmake .

This will open an interface with all the available options.

