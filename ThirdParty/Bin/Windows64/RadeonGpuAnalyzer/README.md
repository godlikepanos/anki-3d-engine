# RGA (Radeon™ GPU Analyzer) #

Radeon GPU Analyzer is a compiler and code analysis tool for Vulkan®, DirectX®, OpenGL® and OpenCL™. Using this product, you can compile high-level source code for a variety of AMD GPU and APU architectures,
independent of the type of GPU/APU that is physically installed on your system.

You can use RGA to produce the following output:
* RDNA™ and GCN ISA disassembly
* Intermediate language disassembly: AMDIL, DXIL and DXBC for DirectX, SPIR-V for Vulkan, LLVM IR for Offline OpenCL
* Hardware resource usage statistics, such as register consumption, static memory allocation and more
* Compiled binaries
* Live register analysis (see http://gpuopen.com/live-vgpr-analysis-radeon-gpu-analyzer/ for more info)
* Control flow graphs
* Build errors and warnings

The RGA package contains both a GUI app and a command-line executable.

The supported modes by the **GUI app** are:
* Vulkan - GLSL/SPIR-V as input, together with the Vulkan pipeline state; compiled through AMD's Vulkan driver
* OpenCL - AMD's LLVM-based Lightning Compiler for OpenCL

The supported modes by the **command-line tool** are:
* DX12 (see https://gpuopen.com/radeon-gpu-analyzer-2-2-direct3d12-compute/ and https://gpuopen.com/radeon-gpu-analyzer-2-3-direct3d-12-graphics/ for more details)
* DX11
* DXR
* Vulkan - compilation of GLSL/SPIR-V together with the API's pipeline state, using AMD's Vulkan driver
* Vulkan Offline - using a static compiler, accepts GLSL/SPIR-V as input. Note: to ensure that the results that RGA provides are accurate and reflect the real-world case, please use the new Vulkan live driver mode (which is also supported in the GUI application).
* OpenCL - AMD's LLVM-based Lightning Compiler for Offline OpenCL
* OpenGL
* AMDIL

## System Requirements ##

* Windows: 10, 64-bit. Visual Studio 2015 or later.
* Linux: Ubuntu 20.04. Build with gcc 4.7.2 or later.
* Vulkan SDK 1.1.97.0 or later. To download the Vulkan SDK, visit https://vulkan.lunarg.com/

To run the tool, you would need to have the AMD Radeon Adrenalin Software (Windows) or amdgpu-pro driver (Linux) installed for all modes, except for the following "offline" modes, which are independent of the driver and hardware:
* Vulkan offline mode
* OpenCL offline mode

For the non-offline modes, it is strongly recommended to run with the latest drivers so that the latest compiler is used and the latest architectures can be targeted.

A specific note for Vulkan mode users:

RGA releases are packaged with the AMD Vulkan driver to enable users who run on machines without an AMD GPU or driver. This is not the case if you build the tool yourself. To enable a custom RGA build on a non-AMD machine, copy the "amdvlk" folder from an RGA release archive
to your build output folder (make sure to place the folder in the same folder hierarchy as in the release archive). Please note that this is a workaround and not the recommended configuration.

## Build Instructions ##

### Building on Windows ###
As a preliminary step, make sure that you have the following installed on your system:
* CMake 3.10 or above. For auto-detecting the Vulkan SDK version 3.7 or above is required.
* Python 2.7 or above
* Qt (in case that you are interested in building the GUI app; you can build the command line executable without Qt). Qt 5.15.2 is recommended.

cd to the build sub-folder, and run:

prebuild.bat --qt <path of Qt's msvc2017_64 folder> --vs 2017

Where <path to Qt's msvc2017_64 folder> is the path to the Qt msvc2017_64 folder, such as C:\Qt\Qt5.15.2\msvc2017_64.

Running the prebuild script will fetch all the dependencies and generate the solution file for Visual Studio.
After successfully running the preuild script, open RGA.sln from build\windows\vs2019 (or vs2017), and build:
* RadeonGPUAnalyzerCLI project for the command line executable
* RadeonGPUAnalyzerGUI project for the GUI app

Some useful options of the prebuild script:
* --vs <VS version>: generate the solution files for a specific Visual Studio version. For example, to target VS 2019, add --vs 2019 to the command.
* --qt <path>: full path to the folder from where you would like the Qt binaries to be retrieved. By default, CMake would try to auto-detect Qt on the system.
* --vk-include and --vk-lib: full paths to where the Vulkan SDK include and Vulkan lib folders. By default, CMake would try to auto-detect the Vulkan SDK on the system.
* --cli-only: only build the command line tool (do not build the GUI app)
* --no-fetch: do not attempt to update the third-party repositories

If you are intending to analyze DirectX 11 shaders using RGA, copy the x64 version of Microsoft's D3D compiler to a subdirectory
named "utils" under the RGA executable's directory (for example, D3DCompiler_47.dll).

-=-

If for some reason you do not want to use the prebuild.bat script, you can also manually fetch the dependencies and generate the solution and project files:
Start by running the FetchDependencies.py script to fetch the solution's dependencies.
To generate the solution file for VS 2017 in x64 configuration, use:

  cmake.exe -G "Visual Studio 15 2017 Win64" <full path to the RGA repo directory>

If you are intending to analyze DirectX shaders using RGA, copy the x64 version of Microsoft's D3D compiler to a subdirectory
named "utils" under the RGA executable's directory (for example, D3DCompiler_47.dll).

### Building on Ubuntu ###
* One time setup:
  * Install the Vulkan SDK (version 1.1.97.0 or above). To download the Vulkan SDK, visit https://vulkan.lunarg.com/
  * sudo apt-get install libboost-all-dev
  * sudo apt-get install gcc-multilib g++-multilib
  * sudo apt-get install libglu1-mesa-dev mesa-common-dev libgtk2.0-dev
  * sudo apt-get install zlib1g-dev libx11-dev:i386
  * Install CMake 3.10 or above. For auto-detecting the Vulkan SDK version 3.7 or above is required.
  * Install python 3.6 (or above)
  * To build the GUI app, you should also have Qt installed

* Build:

   cd to the Build sub-folder

   On Linux, it is recommended to explicitly pass to CMake the location of the Vulkan SDK include and lib directories as well as the location of Qt. For example:

   ./prebuild.sh --qt ~/Qt-5.15.2/5.15.2/gcc_64 --vk-include ~/work/vulkan-sdk/1.1.97.0/x86_64/include/ --vk-lib ~/work/vulkan-sdk/1.1.97.0/x86_64/lib/

   This will fetch all the dependencies and generate the make files.

   Then, cd to the auto-generated subfolder build/linux/make and run make.

   -=-

   If for some reason you do not want to use the prebuild.sh script, you can also manually fetch the dependencies and generate the makefiles:

  * run: python3 fetch_dependencies.py
  * run: cmake –DCMAKE_BUILD_TYPE=Release (or: Debug) <full or relative path to the RGA repo directory>

    It is recommended to create a directory to hold all build files, and launch cmake from that directory.

    For example:
    * cd to the RGA repo directory
    * mkdir _build
    * cd _build
    * cmake –DCMAKE_BUILD_TYPE=Release ../
  * run: make

## Running ##
### GUI App ###
Run the RadeonGPUAnalyzerGUI executable. The app provides a quickstart guide and a help manual under Help.

### Command Line Executable ###

Run the rga executable.

* Usage:
  * General: rga -h
  * DirectX 12: rga -s dx12 -h
  * DirectX 11: rga -s dx11 -h
  * DirectX Raytracing: rga -s dxr -h

    Note: RGA's DX11 mode requires Microsoft's D3D Compiler DLL in runtime. If you copy the relevant D3D Compiler DLL to the utils
    subdirectory under the executable's directory, RGA will use that DLL in runtime. The default D3D compiler that RGA public releases ship with
    is d3dcompiler_47.dll.
  * OpenGL:  rga -s opengl -h
  * OpenCL offline:  rga -s opencl -h
  * Vulkan live-driver:  rga -s vulkan -h
  * Vulkan offline - glsl:  rga -s vk-offline -h
  * Vulkan offline - SPIR-V binary input:  rga -s vk-offline-spv -h
  * Vulkan offline - SPIRV-V textual input:  rga -s vk-offline-spv-txt -h
  * AMD IL: rga -s amdil -h

## Support ##
For support, please visit the RGA repository github page: https://github.com/GPUOpen-Tools/RGA

## Style and Format Change ##
The source code of this product is being reformatted to follow the Google C++ Style Guide https://google.github.io/styleguide/cppguide.html

In the interim you may encounter a mix of both an older C++ coding style, as well as the newer Google C++ Style.

Please refer to the _clang-format file in the root directory of the product for additional style information.

## License ##
Radeon GPU Analyzer is licensed under the MIT license. See the License.txt file for complete license information.

## Copyright information ##
Please see RGA_NOTICES.txt for copyright and third party license information.
