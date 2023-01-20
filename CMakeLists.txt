CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

include(ProcessorCount)

PROJECT(AnKi)

################################################################################
# Funcs                                                                        #
################################################################################

function(anki_install_executable EXE)
	if(NOT ANDROID)
		add_custom_command(TARGET ${EXE} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Bin
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXE}> ${CMAKE_BINARY_DIR}/Bin)
	endif()
endfunction()

macro(anki_new_executable)
	if(NOT ANDROID)
		add_executable(${ARGV})
		set_target_properties(${ARGV0} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
	else()
		set(_SKIP TRUE)
		foreach(ARG ${ARGV})
			if(_SKIP)
				set(_SKIP FALSE)
			else()
				list(APPEND _TMP_LIST ${ARG})
			endif()
		endforeach()

		add_library(${ARGV0} SHARED ${_TMP_LIST})
	endif()
endmacro()

################################################################################
# Determin the system to build for. Do that first                              #
################################################################################

if(WIN32)
	if(NOT WINDOWS)
		set(WINDOWS TRUE)
		message("++ Building for windows")
	endif()
elseif(UNIX AND NOT APPLE)
	if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
		set(LINUX TRUE)
		message("++ Building for Linux")
	elseif(ANDROID)
		message("++ Building for Android")
	else()
		message(FATAL_ERROR "Unknown unix")
	endif()
elseif(APPLE)
	if(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*")
		set(MACOS TRUE)
		message("++ Building for MacOS")
	else()
		message(FATAL_ERROR "Unknown apple")
	endif()
else()
	message(FATAL_ERROR "Unknown system")
endif()

# Indentify compiler
set(GCC FALSE)
set(CLANG FALSE)
set(CLANG_WINDOWS FALSE)

if(${CMAKE_C_COMPILER_ID} MATCHES "GNU")
	set(GCC TRUE)
	message("++ Compiler identified as GCC")
endif()

if(${CMAKE_C_COMPILER_ID} MATCHES "Clang")
	message("++ Compiler identified as Clang")
	set(CLANG TRUE)
	if(WINDOWS)
		# It's clang for windows
		message("++ Compiler identified as Clang for Windows")
		set(CLANG_WINDOWS TRUE)
	endif()
endif()

if(MSVC)
	message("++ Compiler identified as MSVC")
endif()

# Identify the target system
set(X86 FALSE)
set(ARM FALSE)
if(GCC OR CLANG)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE target_arch)

	if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch" OR ${target_arch} MATCHES "aarch")
		set(ARM TRUE)
	elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86.*" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64.*")
		set(X86 TRUE)
	else()
		message(FATAL_ERROR "Couldn't find the target architecture from: ${target_arch} or ${CMAKE_SYSTEM_PROCESSOR}")
	endif()
elseif(MSVC)
	set(X86 TRUE)
else()
	message(FATAL_ERROR "Couldn't find the target architecture")
endif()

if(X86)
	message("++ Target architecture is X86")
elseif(ARM)
	message("++ Target architecture is ARM")
endif()

################################################################################
# Configuration                                                                #
################################################################################

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ANKI_EXTRA_CHECKS "Debugging checks (assertions)" OFF)
option(ANKI_LTO "LTO compilation" OFF)

option(ANKI_BUILD_TOOLS "Build tools" ON)
option(ANKI_BUILD_TESTS "Build unit tests" OFF)
option(ANKI_BUILD_SANDBOX "Build sandbox application" ON)
option(ANKI_BUILD_SAMPLES "Build sample applications" ON)

option(ANKI_STRIP "Strip the symbols from the executables" OFF)

option(ANKI_TRACE "Enable performance tracing. Small overhead" OFF)
if(ANKI_TRACE)
	set(_ANKI_ENABLE_TRACE 1)
else()
	set(_ANKI_ENABLE_TRACE 0)
endif()

option(ANKI_SIMD "Enable SIMD optimizations" ON)
option(ANKI_ADDRESS_SANITIZER "Enable address sanitizer (-fsanitize=address)" OFF)
option(ANKI_HEADLESS "Build a headless application" OFF)
option(ANKI_SHADER_FULL_PRECISION "Build shaders with full precision" OFF)
set(ANKI_OVERRIDE_SHADER_COMPILER "" CACHE FILEPATH "Set the ShaderCompiler to be used to compile all shaders")
option(ANKI_DLSS "Integrate DLSS if supported" OFF)

# Take a wild guess on the windowing system
if(ANKI_HEADLESS)
	set(SDL FALSE)
elseif(LINUX)
	set(SDL TRUE)
elseif(WINDOWS)
	set(SDL TRUE)
elseif(ANDROID)
	set(SDL FALSE)
elseif(MACOS)
	set(SDL TRUE)
else()
	message(FATAL_ERROR "Couldn't determine the window backend. You need to specify it manually.")
endif()

set(ANKI_GR_BACKEND "VULKAN" CACHE STRING "The graphics API to use (VULKAN or GL)")

if(${ANKI_GR_BACKEND} STREQUAL "GL")
	set(GL TRUE)
	set(VULKAN FALSE)
	set(VIDEO_VULKAN TRUE) # Set for the SDL2 to pick up
else()
	set(GL FALSE)
	set(VULKAN TRUE)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
	message(FATAL_ERROR "You need to define CMAKE_BUILD_TYPE")
endif()

################################################################################
# Common compiler & linker flags                                               #
################################################################################

if(MINGW)
	add_compile_options(-mconsole)
endif()

add_definitions(
	-DSPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
	-DANKI_BUILD
	-DIMGUI_USER_CONFIG=<AnKi/Ui/ImGuiConfig.h>)

if(NOT MSVC)
	# When building AnKi define this special flag
	if(NOT CLANG_WINDOWS)
		add_compile_options(-fPIC)
	endif()

	add_compile_options(-fno-exceptions)

	if(GCC)
		add_compile_options(-static-libstdc++)
	endif()

	if(X86)
		add_compile_options(-msse4)
	endif()

	if(ANKI_LTO)
		add_compile_options(-flto)
		set(LINKER_FLAGS "${LINKER_FLAGS} -flto ")
	endif()

	if(ANKI_STRIP)
		set(LINKER_FLAGS "${LINKER_FLAGS} -s ")
		add_compile_options(-s)
	endif()

	if(ANKI_ADDRESS_SANITIZER)
		add_compile_options(-fsanitize=address)
	endif()

	if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
		add_compile_options(-O3)
		add_definitions(-DNDEBUG)
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		add_compile_options(-O3 -g3 -fno-omit-frame-pointer)
		set(LINKER_FLAGS "${LINKER_FLAGS} -rdynamic ") # For backtrace()
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		add_compile_options(-O0 -g3)
		set(LINKER_FLAGS "${LINKER_FLAGS} -rdynamic ") # For backtrace()
	else()
		message(FATAL_ERROR "Wrong CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
	endif()

	# Set the flags to cmake now
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
else()
	#ProcessorCount(PC)
	add_compile_options(/MP)

	if(${CMAKE_BUILD_TYPE} STREQUAL "Release" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		#add_definitions(/Ox)
	endif()

	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS=1 # Disable some string function warnings
		-D_ITERATOR_DEBUG_LEVEL=0)

	# Full paths in compiler diagnostics else you can't click on visual studio have it open the file+line
	add_compile_options(/FC)
endif()

# Use LLD or gold linker
if(UNIX AND NOT APPLE)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=lld -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
	if("${ld_version}" MATCHES "compatible with GNU linkers" AND "${ld_version}" MATCHES "LLD")
		message("++ Will use LLD linker")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -Wl,--disable-new-dtags")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld -Wl,--disable-new-dtags")
	else()
		execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
		if("${ld_version}" MATCHES "GNU gold")
			message("++ Will use gold linker")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
		endif()
	endif()
endif()

################################################################################
# Thirdparty                                                                   #
################################################################################
set(ANKI_EXTERN_SUB_DIRS TinyXml2 Lua ZLib Bullet ImGui MeshOptimizer SpirvCross)

# Bullet config
option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_BULLET3 OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_UNIT_TESTS OFF)

if((LINUX OR MACOS OR WINDOWS) AND GL)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} GLEW)
endif()

# SDL
if(SDL)
	message("++ Configuring SDL2")
	add_subdirectory(ThirdParty/Sdl2)
	message("++ End configuring SDL2")

	# Include first the build directory.
	set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Sdl2/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Sdl2/include")
endif()

# glslang
message("++ Configuring glslang")
add_subdirectory(ThirdParty/Glslang)
message("++ End configuring glslang")

if(LINUX OR WINDOWS)
	message("++ Configuring reproc")
	set(REPROC++ OFF)
	add_subdirectory(ThirdParty/Reproc)
	message("++ End configuring reproc")
endif()

foreach(TMP ${ANKI_EXTERN_SUB_DIRS})
	add_subdirectory(ThirdParty/${TMP})
endforeach()

if(ANDROID)
	add_library(AnKiAndroidNativeGlue ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

	add_subdirectory(ThirdParty/HwcPipe)
endif()

# DLSS
if(ANKI_DLSS)
	add_subdirectory(ThirdParty/DlssSdk)
	set(_ANKI_DLSS_ENABLED 1)
else()
	set(_ANKI_DLSS_ENABLED 0)
endif()

################################################################################
# AnKi                                                                         #
################################################################################

# Revision
find_package(Git)

if(GIT_FOUND)
	execute_process(COMMAND
		"${GIT_EXECUTABLE}" log -1 --date=short --format=%h
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		OUTPUT_VARIABLE GIT_COMMIT
		ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

	set(ANKI_REVISION "\"${GIT_COMMIT}\"")
else()
	set(ANKI_REVISION "\"unknown\"")
endif()

# Doxygen
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Docs/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

find_package(Doxygen)

if(DOXYGEN_FOUND)
	message("++ Doxygen found")

	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

# Config.h
set(ANKI_VERSION_MAJOR 0)
set(ANKI_VERSION_MINOR 1)
message("++ AnKi version: ${ANKI_VERSION_MAJOR}.${ANKI_VERSION_MINOR}")

if(ANKI_EXTRA_CHECKS)
	set(_ANKI_EXTRA_CHECKS 1)
else()
	set(_ANKI_EXTRA_CHECKS 0)
endif()

if(ANKI_SIMD)
	set(_ANKI_ENABLE_SIMD 1)
else()
	set(_ANKI_ENABLE_SIMD 0)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(ANKI_DEBUG_SYMBOLS 1)
	set(ANKI_OPTIMIZE 0)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	set(ANKI_DEBUG_SYMBOLS 0)
	set(ANKI_OPTIMIZE 1)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	set(ANKI_DEBUG_SYMBOLS 1)
	set(ANKI_OPTIMIZE 1)
else()
	message(FATAL_ERROR "Wrong CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

if(ANKI_BUILD_TESTS)
	set(ANKI_TESTS 1)
else()
	set(ANKI_TESTS 0)
endif()

if(ANKI_HEADLESS)
	set(_ANKI_WINDOWING_SYSTEM 0)
elseif(SDL)
	set(_ANKI_WINDOWING_SYSTEM 1)
else()
	set(_ANKI_WINDOWING_SYSTEM 2)
endif()

configure_file("AnKi/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/AnKi/Config.h")

# Include & lib directories
include_directories(
	"ThirdParty/Khronos"
	"${CMAKE_CURRENT_BINARY_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Bullet/src"
	"ThirdParty"
	"ThirdParty/ZLib"
	${CMAKE_CURRENT_SOURCE_DIR})

if(SDL2_INCLUDE_DIRS)
	include_directories("${SDL2_INCLUDE_DIRS}")
endif()

if(LINUX OR MACOS OR WINDOWS)
	include_directories("ThirdParty/GLEW/include")
else()
	#include_directories("ThirdParty/GLES3/include")
endif()

if(ANDROID)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
endif()

if(LINUX)
	set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} pthread dl)

	if(SDL)
		set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} X11-xcb)
	endif()
endif()

if(ANKI_DLSS)
	set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} AnKiNgx)
endif()

# AnKi compiler flags (Mainly warnings)
if(NOT MSVC)
	add_compile_options(
		-pedantic
		-Wno-unknown-warning-option
		-Wall
		-W
		-Wextra
		-Wstrict-aliasing
		-Wwrite-strings
		-Wunused
		-Wundef
		-Wno-ignored-attributes
		-Wno-implicit-fallthrough
		-Wunused-result
		-Wconversion
		-Wno-sign-conversion
		-Wno-keyword-macro
		-Wno-string-conversion
		-Wno-class-memaccess
		-Wunused-variable)

	set(CMAKE_CXX_STANDARD 20)
else()
	string(REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

	add_compile_options(
		/std:c++20
		/W4
		/wd4324
		/wd4456
		/wd4127
		/wd4457)
endif()

# Add AnKi sub libraries
add_subdirectory(AnKi)

if(MSVC)
    add_library(AnKi INTERFACE AnKi.natvis)
else()
    add_library(AnKi INTERFACE)
endif()

target_link_libraries(AnKi INTERFACE AnKiCore ${THIRD_PARTY_LIBS})
add_dependencies(AnKi AnKiShaders)

################################################################################
# AnKi extra                                                                   #
################################################################################
if(ANKI_BUILD_TESTS)
	add_subdirectory(Tests)
endif()

if(ANKI_BUILD_TOOLS)
	add_subdirectory(Tools)
endif()

if(ANKI_BUILD_SANDBOX)
	add_subdirectory(Sandbox)
endif()

if(ANKI_BUILD_SAMPLES)
	add_subdirectory(Samples)
endif()

