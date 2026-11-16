# CMake toolchain file to cross-compile for Windows x64 from Linux using clang-cl. Run Tools/Build/FetchXwin.sh first

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64) # The root CMakeLists.txt reads it to detect X86

# Ask clang where its install is. The LLVM tools live next to it and that location differs between distros
find_program(_CLANG clang REQUIRED)
execute_process(COMMAND ${_CLANG} -print-prog-name=clang-cl OUTPUT_VARIABLE _CLANG_CL OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT EXISTS "${_CLANG_CL}")
	message(FATAL_ERROR "clang-cl not found next to ${_CLANG}")
endif()
get_filename_component(_LLVM_BIN ${_CLANG_CL} DIRECTORY)

set(CMAKE_C_COMPILER ${_LLVM_BIN}/clang-cl)
set(CMAKE_CXX_COMPILER ${_LLVM_BIN}/clang-cl)
set(CMAKE_LINKER ${_LLVM_BIN}/lld-link)
set(CMAKE_AR ${_LLVM_BIN}/llvm-lib)
set(CMAKE_RC_COMPILER ${_LLVM_BIN}/llvm-rc)
set(CMAKE_MT ${_LLVM_BIN}/llvm-mt)

set(_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty/Xwin/sdk")
set(CMAKE_C_FLAGS_INIT "--target=x86_64-pc-windows-msvc /winsysroot \"${_SYSROOT}\"")
set(CMAKE_CXX_FLAGS_INIT "--target=x86_64-pc-windows-msvc /winsysroot \"${_SYSROOT}\"")
set(CMAKE_EXE_LINKER_FLAGS_INIT "\"/winsysroot:${_SYSROOT}\"")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "\"/winsysroot:${_SYSROOT}\"")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "\"/winsysroot:${_SYSROOT}\"")

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL) # Wine and Proton have no debug CRT DLLs

set(CMAKE_CROSSCOMPILING_EMULATOR wine) # Runs ShaderCompiler.exe during the build
