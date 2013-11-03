
AnKi 3D engine is a Linux opensource game engine buld using OpenGL.

=======
License
=======

AnKi's license is GPLv3. This practicaly means that AnKi as a whole or parts of
it can be used only in software that is licensed under GPLv3 or GPLv3 compatible
licenses.

In the future the license will change into a BSD-like to allow more freedom.

=============
Building AnKi
=============

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

===============================
Supported hardware and software
===============================

AnKi has 3 codepaths: 

- OpenGL 3.3 core
- OpenGL 4.4
- OpenGL ES 3.0

It's been tested on nVidia HW with some less than a year old nVidia 
proprietary drivers. Different HW and drivers have not been tested yet and they
are supported only in theory.

It's been known to build and run on Ubuntu 12.04 64bit.
