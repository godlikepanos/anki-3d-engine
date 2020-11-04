CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

include(ProcessorCount)

PROJECT(anki)

################################################################################
# Funcs                                                                        #
################################################################################

macro(installExecutable exe)
	add_custom_command(TARGET ${exe} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${exe}> ${CMAKE_BINARY_DIR}/bin)
endmacro()

macro(addAnkiSourceFiles)
	foreach(f ${ARGV})
		set(AK_SOURCES "${AK_SOURCES} ${f}")
	endforeach()

	set(AK_SOURCES ${AK_SOURCES} PARENT_SCOPE)
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
		if(NOT ANDROID)
			set(LINUX TRUE)
			message("++ Building for Linux")
		else()
			message("++ Building for Android")
		endif()
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

if(${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
	set(GCC TRUE)
else()
	set(GCC FALSE)
endif()

if(${CMAKE_C_COMPILER_ID} MATCHES "Clang")
	set(CLANG TRUE)
else()
	set(CLANG FALSE)
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
option(ANKI_BUILD_BENCH "Build benchmark application" OFF)
option(ANKI_BUILD_SAMPLES "Build sample applications" ON)

option(ANKI_STRIP "Srip the symbols from the executables" OFF)

option(ANKI_TRACE "Enable performance tracing. Small overhead" OFF)
if(ANKI_TRACE)
	set(_ANKI_ENABLE_TRACE 1)
else()
	set(_ANKI_ENABLE_TRACE 0)
endif()

set(ANKI_CPU_ADDR_SPACE "0" CACHE STRING "The CPU architecture (0 or 32 or 64). If zero go native")

option(ANKI_SIMD "Enable or not SIMD optimizations" ON)
option(ANKI_ADDRESS_SANITIZER "Enable address sanitizer (-fsanitize=address)" OFF)

# Take a wild guess on the windowing system
if(LINUX)
	set(_WIN_BACKEND "SDL")
	set(SDL TRUE)
elseif(WINDOWS)
	set(_WIN_BACKEND "SDL")
	set(SDL TRUE)
elseif(ANDROID)
	set(_WIN_BACKEND "ANDROID")
elseif(MACOS)
	set(_WIN_BACKEND "SDL")
	set(SDL TRUE)
else()
	message(FATAL_ERROR "Couldn't determine the window backend. You need to specify it manually")
endif()

set(ANKI_GR_BACKEND "VULKAN" CACHE STRING "The graphics API (VULKAN or GL)")

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

if(${ANKI_CPU_ADDR_SPACE} STREQUAL "0" OR ${ANKI_CPU_ADDR_SPACE} STREQUAL "64")
	set(ARCH_64 TRUE)
	set(ARCH_32 FALSE)
else()
	set(ARCH_64 FALSE)
	set(ARCH_32 TRUE)
endif()

################################################################################
# Compiler & linker flags                                                      #
################################################################################

if(MINGW)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -mconsole ")
endif()

add_definitions(-DSPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
add_definitions(-DANKI_BUILD)

# ImGUI
add_definitions(-DIMGUI_USER_CONFIG=<anki/ui/ImGuiConfig.h>)

if(NOT MSVC)
	# When building AnKi define this special flag
	add_definitions("-fPIC")

	add_definitions("-fno-exceptions")

	if(GCC AND NOT CLANG)
		add_definitions("-static-libstdc++")
	endif()

	if(LINUX OR MACOS OR WINDOWS)
		add_definitions("-msse4")
	else()
		add_definitions("-mfpu=neon")
	endif()

	if(ANKI_LTO)
		add_definitions("-flto ")
		set(LINKER_FLAGS "${LINKER_FLAGS} -flto ")
	endif()

	if(ANKI_STRIP)
		set(LINKER_FLAGS "${LINKER_FLAGS} -s ")
		add_definitions("-s")
	endif()

	if(ANKI_ADDRESS_SANITIZER)
		add_definitions("-fsanitize=address ")
	endif()

	if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
		add_definitions("-O3 -DNDEBUG")
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		add_definitions("-O3 -g3 -fno-omit-frame-pointer")
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		add_definitions("-O0 -g3")
	else()
		message(FATAL_ERROR "Wrong CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
	endif()

	# Set the flags to cmake now
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
else()
	#ProcessorCount(PC)
	#add_definitions("/MP${PC}")

	if(${CMAKE_BUILD_TYPE} STREQUAL "Release" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		#add_definitions("/Ox")
	endif()
endif()

# Use gold linker
if(UNIX AND NOT APPLE)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
	if("${ld_version}" MATCHES "GNU gold")
		message("++ Will use gold linker")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
	endif()
endif()

################################################################################
# Thirdparty                                                                   #
################################################################################
set(ANKI_EXTERN_SUB_DIRS tinyxml2 lua z bullet imgui meshoptimizer)

# Bullet config
option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_BULLET3 OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_EXTRAS OFF)

if((LINUX OR MACOS OR WINDOWS) AND GL)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} GLEW)
endif()

set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} SPIRV-Cross)

if(VULKAN)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} volk)
endif()

# SDL
if(SDL)
	message("++ Configuring SDL2")
	add_subdirectory(thirdparty/SDL2)
	message("++ End configuring SDL2")

	# Include first the build directory.
	set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/SDL2/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2/include")
else()
	set(SDL2_INCLUDE_DIRS "")
endif()

# freetype
message("++ Configuring freetype")
add_subdirectory(thirdparty/freetype)
message("++ End configuring freetype")

# glslang
message("++ Configuring glslang")
add_subdirectory(thirdparty/glslang)
message("++ End configuring glslang")

# tinyexpr
add_subdirectory(thirdparty/tinyexpr)

foreach(TMP ${ANKI_EXTERN_SUB_DIRS})
	add_subdirectory(thirdparty/${TMP})
endforeach()

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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)

find_package(Doxygen)

if(DOXYGEN_FOUND)
	message("++ Doxygen found")

	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
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

configure_file("anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")

# Include & lib directories
include_directories(
	"thirdparty/tinyxml2/include"
	"thirdparty/lua"
	"thirdparty/imgui"
	"thirdparty/z"
	"${SDL2_INCLUDE_DIRS}"
	"thirdparty/freetype/include"
	"${CMAKE_CURRENT_BINARY_DIR}/thirdparty/freetype/include/freetype2"
	"thirdparty/khronos"
	"${CMAKE_CURRENT_BINARY_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glslang"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyexpr"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/bullet/src"
	"thirdparty"
	${CMAKE_CURRENT_SOURCE_DIR})

if(LINUX OR MACOS OR WINDOWS)
	include_directories("thirdparty/GLEW/include")
else()
	#include_directories("thirdparty/GLES3/include")
endif()

if(ANDROID)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
endif()

# AnKi compiler flags (Mainly warnings)
if(NOT MSVC)
	add_definitions("-pedantic -Wno-unknown-warning-option -Wall -W -Wextra -Wstrict-aliasing -Wwrite-strings -Wunused "
		"-Wno-unused-parameter -Wundef -Wno-ignored-attributes -Wno-implicit-fallthrough -Wunused-result "
		"-Wconversion -Wno-sign-conversion -Wno-keyword-macro -Wno-string-conversion -Wno-class-memaccess "
		"-Wunused-variable -std=c++14")
else()
	add_definitions("/wd4996 /wd4244 /wd4262 /wd4267 /wd26495 /wd26439")
endif()

# Set platform specific
if(LINUX)
	if(GL)
		set(THIRD_PARTY_LIBS ${ANKI_GR_BACKEND} ankiglew)
	else()
		set(THIRD_PARTY_LIBS ankivolk)
		if(SDL)
			set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} X11-xcb)
		else()
			message(FATAL_ERROR "Unhandled case")
		endif()
	endif()

	set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} pthread dl)
elseif(MACOS)
	find_package(OpenGL REQUIRED)
	set(THIRD_PARTY_LIBS ${OPENGL_LIBRARIES} ankiglew pthread)
elseif(ANDROID)
	set(THIRD_PARTY_LIBS GLESv3 EGL log android)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
	set(_SYS_SRC "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
elseif(WINDOWS)
	if(GL)
		set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} ankiglew opengl32)
	else()
		set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} ankivolk)
	endif()

	set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} version Imm32 Winmm DbgHelp)
else()
	message(FATAL_ERROR "Unhandled case")
endif()

if(SDL)
	set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} SDL2-static)
endif()

set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} BulletSoftBody BulletDynamics BulletCollision LinearMath
	ankispirvcross ankitinyxml2 ankilua ankimeshoptimizer ankiz glslang SPIRV OGLCompiler OSDependent ankitinyexpr
	ankiimgui)

# Add anki sub libraries
add_subdirectory(anki)

separate_arguments(AK_SOURCES)
add_library(anki ${AK_SOURCES})
target_compile_definitions(anki PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(anki ${THIRD_PARTY_LIBS})

################################################################################
# AnKi extra                                                                   #
################################################################################
if(ANKI_BUILD_TESTS)
	add_subdirectory(tests)
endif()

if(ANKI_BUILD_TOOLS)
	add_subdirectory(tools)
endif()

if(ANKI_BUILD_SANDBOX)
	add_subdirectory(sandbox)
endif()

if(ANKI_BUILD_SAMPLES)
	add_subdirectory(samples)
endif()

if(ANKI_BUILD_BENCH)
	add_subdirectory(bench)
endif()

