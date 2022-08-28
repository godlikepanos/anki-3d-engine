Build from GitHub Sources
+++++++++++++++++++++++++

Compressonator comes with pre-built binaries for all of our stable releases:

https://github.com/GPUOpen-Tools/Compressonator/releases

which includes command line tools, GUI and binaries for use in developer applications.

The following build instructions are provided for developers interested in building the latest sources available on `Compressonator GitHub <https://github.com/GPUOpen-Tools/Compressonator>`_ or from the source code downloaded from the `releases <https://github.com/GPUOpen-Tools/Compressonator/releases>`_ page.

.. code-block:: console

    git clone --recursive https://github.com/GPUOpen-Tools/Compressonator.git

to get all the source code including the submodules contents.


Build Instructions for Compressonator SDK Libs
==============================================

VS2017 project soultion files are provided to build SDK libs in the folder compressonator/vs2017


Build Instructions for Compressonator GUI and CLI applications
==============================================================

Building on Windows
As a preliminary step, make sure that you have the following installed on your system:

- CMake 3.15 or above
- Vulkan SDK version 1.2.141.2 or above is required
- Python 3.6 or above
- Qt 5.12.6 is recommended

Once installed users can run any of the following to build:

Documents
---------
cd to the Compressonator folder, and run:


.. code-block:: console

    call scripts/windows_build_docs.bat

Applications
-------------

.. code-block:: console

    call scripts/windows_build_apps.bat

This call will first download additional dependencies used into a "common" folder above the compressonator source folder. It calls a python script called "fetch_dependencies.py" and cmake to build a vs2017 solution file into a "build" folder


Building all sdk libs
---------------------
.. code-block:: console

    call scripts/windows_build_sdk.bat

If you have Visual Studio 2017, build solutions can also be setup by running the vsbuild.bat file that calls fetch_dependencies.py and cmake.

Linux Builds Ubuntu 18.04
--------------------------

As a preliminary step, make sure that you have the following packages installed on your system:

- CMake 3.10 or above
- Vulkan SDK version 1.2.141.2 or above is required
- Python 3.6 or above
- Qt 5.9.2 (Qt 5.12.6 is recommended)


Additional packages are required and can be installed using the shell script: initsetup_ubuntu.sh

Then run the following in the compressonator/scripts folder

.. code-block:: console

    python3 fetch_dependencies.py --no-hooks

To build applications use

.. code-block:: console

    cd compressonator
    export VULKAN_SDK_VER=1.2.141.2
    export VULKAN_SDK=/opt/VulkanSDK/$VULKAN_SDK_VER/
    QT_ROOT=/opt/Qt/Qt5.9.2/5.9.2/gcc_64
    cmake -DQT_PACKAGE_ROOT=$QT_ROOT .
    make

The QT_ROOT and VULKAN_SDK should be set to your package install folder

To build documents use

.. code-block:: console

    set -x
    cd compressonator/docs
    make -j 4 clean
    make -j 4 html

