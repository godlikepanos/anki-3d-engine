CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

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

option(ANKI_EXTRA_CHECKS "Debugging checks (assertions)" OFF)
option(ANKI_LTO "LTO compilation" OFF)

option(ANKI_BUILD_TOOLS "Build tools" OFF)
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

set(ANKI_GR_BACKEND "GL" CACHE STRING "The graphics API (GL or VULKAN)")

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

set(CXX_FLAGS "")
set(COMPILER_FLAGS "")
set(LINKER_FLAGS "")

add_definitions(-D_NEWTON_STATIC_LIB -D_CUSTOM_JOINTS_STATIC_LIB)

if(NOT MSVC)
	if(ARCH_64)
		add_definitions(-D_POSIX_VER_64)
	else()
		add_definitions(-D_POSIX_VER_32)
	endif()
endif()

# Newton wants that
if(MINGW AND ARCH_64)
	add_definitions(-D_MINGW_64_VER)
elseif(MINGW AND ARCH_32)
	add_definitions(-D_MINGW_32_VER)
endif()

if(MINGW)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -mconsole ")
endif()

add_definitions(-DGLEW_NO_GLU)
add_definitions(-DSPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
add_definitions(-DANKI_BUILD)

if(NOT MSVC)
	# When building AnKi define this special flag
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -fPIC ")

	if(NOT ANKI_CPU_ADDR_SPACE STREQUAL "0")
		set(LINKER_FLAGS "${LINKER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
	endif()

	set(COMPILER_FLAGS "${COMPILER_FLAGS} -fno-exceptions ")

	if(GCC AND NOT CLANG)
		set(CXX_FLAGS "${CXX_FLAGS} -static-libstdc++ ")
	endif()

	if(ANKI_SIMD)
		if(LINUX OR MACOS OR WINDOWS)
			set(COMPILER_FLAGS "${COMPILER_FLAGS} -msse4 ")
		else()
			set(COMPILER_FLAGS "${COMPILER_FLAGS} -mfpu=neon ")
		endif()
	endif()

	if(ANKI_LTO)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -flto ")
		set(LINKER_FLAGS "${LINKER_FLAGS} -flto ")
	endif()

	if(ANKI_STRIP)
		set(LINKER_FLAGS "${LINKER_FLAGS} -s ")
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -s ")
	endif()

	if(ANKI_ADDRESS_SANITIZER)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fsanitize=address ")
	endif()

	if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -O3 -DNDEBUG ")

		set(CMAKE_CXX_FLAGS_RELEASE "")
		set(CMAKE_C_FLAGS_RELEASE "")
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -O3 -g3 ")

		set(CMAKE_CXX_FLAGS_RELWITHDBGINFO "")
		set(CMAKE_C_FLAGS_RELWITHDBGINFO "")
	elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -O0 -g3 ")

		set(CMAKE_CXX_FLAGS_DEBUG "")
		set(CMAKE_C_FLAGS_DEBUG "")
	else()
		message(FATAL_ERROR "Wrong CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
	endif()

	# Set the flags to cmake now
	set(CMAKE_CXX_FLAGS "${CXX_FLAGS} ${COMPILER_FLAGS}")
	set(CMAKE_C_FLAGS "${COMPILER_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
endif()

################################################################################
# Thirdparty                                                                   #
################################################################################
set(ANKI_EXTERN_SUB_DIRS tinyxml2 lua z newton)

if((LINUX OR MACOS OR WINDOWS) AND GL)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} GLEW)
endif()

set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} SPIRV-Cross)

if(ANKI_BUILD_TOOLS)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} assimp)
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

configure_file("src/anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")

# Include & lib directories
include_directories("src"
	"thirdparty/tinyxml2/include"
	"thirdparty/lua"
	"thirdparty/z"
	"${SDL2_INCLUDE_DIRS}"
	"thirdparty/freetype/include"
	"${CMAKE_CURRENT_BINARY_DIR}/thirdparty/freetype/include/freetype2"
	"thirdparty/newton/coreLibrary_300/source/newton"
	"thirdparty/newton/packages/dCustomJoints"
	"thirdparty/newton/packages/dContainers"
	"thirdparty/newton/packages/dMath"
	"thirdparty/newton/packages/thirdParty/timeTracker/"
	"thirdparty/khronos"
	"${CMAKE_CURRENT_BINARY_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glslang"
	"thirdparty")

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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -W -Wextra -Wstrict-aliasing -Wwrite-strings -Wunused -Wunused-variable -Wno-unused-parameter -Wundef -Wno-ignored-attributes -Wno-implicit-fallthrough -Wunused-result -std=c++14")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4244 /wd4262 /wd4267")
endif()

# Set platform specific
if(LINUX)
	if(GL)
		set(THIRD_PARTY_LIBS ${ANKI_GR_BACKEND} ankiglew)
	else()
		set(THIRD_PARTY_LIBS vulkan)
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
		if(NOT DEFINED ENV{VULKAN_SDK})
			message(FATAL_ERROR "You need to have VULKAN SDK installed and the VULKAN_SDK env variable set")
		endif()

		link_directories($ENV{VULKAN_SDK}/Bin)
		set(THIRD_PARTY_LIBS vulkan-1)
	endif()

	set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} version Imm32 Winmm DbgHelp)
else()
	message(FATAL_ERROR "Unhandled case")
endif()

if(SDL)
	set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} SDL2-static)
endif()

set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} ankispirvcross)

set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} glslang SPIRV OGLCompiler OSDependent)

# Add anki sub libraries
set(ANKI_SUB_DIRS core script renderer scene ui event input physics resource misc gr collision math util)
foreach(TMP ${ANKI_SUB_DIRS})
	add_subdirectory(src/anki/${TMP})
endforeach()

separate_arguments(AK_SOURCES)
add_library(anki ${AK_SOURCES})
target_link_libraries(anki ankitinyxml2 ankilua ankiz ankinewton ${THIRD_PARTY_LIBS})

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

