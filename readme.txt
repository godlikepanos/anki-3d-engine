
AnKi 3D engine is a Linux opensource game engine buld using OpenGL.

=============
Building AnKi
=============

AnKi's build system is using CMake. A great effort was made to keep the number 
of external dependencies to minimum so the only prerequisites are the following:

- X11 development files (Package name under Ubuntu: libx11-dev)
- Mesa GL development files (Package name under Ubuntu: mesa-common-dev)

AnKi is using the C++11 standard so the supported compilers are:

- GCC 4.7 or greater 
- clang 3.2 or greater 

To build the release version:

- cd <path_to_anki>/build
- cmake -DCMAKE_BUILD_TYPE=Release ..
- make

To view and configure the build options you can use ccmake tool:

- cd <path_to_anki>/build
- cmake -DCMAKE_BUILD_TYPE=Release ..
- ccmake .

This will open an interface with all the available options.

===============================
Supported hardware and software
===============================

AnKi has 3 codepaths: 

- OpenGL 3.x core
- OpenGL 4.4
- OpenGL ES 3.0

It's been tested on nVidia HW with some less than a year old nVidia 
proprietary drivers. Different HW and drivers have not been tested yet and they
are supported only in theory.

It's been known to build and run on Ubuntu 12.04 64bit.
