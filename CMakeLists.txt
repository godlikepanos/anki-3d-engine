CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(anki)

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

################################################################################
# Configuration                                                                #
################################################################################

option(ANKI_OPTIMIZE "Optimize" ON)
option(ANKI_DEBUG_SYMBOLS "Debug symbols" OFF)
option(ANKI_DEBUG "Debugging checks" OFF)

option(ANKI_BUILD_TOOLS "Build tools" ON)
option(ANKI_BUILD_TESTS "Build unit tests" OFF)
option(ANKI_BUILD_TESTAPP "Build test application" ON)
option(ANKI_BUILD_BENCH "Build benchmark application" OFF)

option(ANKI_WITH_GPERFTOOLS_PROF "Link with gperftools profiler" OFF)

option(ANKI_STRIP "Srip the symbols from the executables" OFF)

option(ANKI_ENABLE_COUNTERS "Enable performance counters. Small overhead" OFF)
if(ANKI_ENABLE_COUNTERS)
	set(_ANKI_ENABLE_COUNTERS 1)
else()
	set(_ANKI_ENABLE_COUNTERS 0)
endif()

set(ANKI_CPU_ADDR_SPACE "0" CACHE STRING "The CPU architecture (0 or 32 or 64). If zero go native")

option(ANKI_ENABLE_SIMD "Enable or not SIMD optimizations" ON)

if(ANKI_ENABLE_SIMD)
	set(_ANKI_ENABLE_SIMD 1)
else()
	set(_ANKI_ENABLE_SIMD 0)
endif()

# Take a wild guess on the windowing system
if(LINUX)
	set(_WIN_BACKEND "SDL")
elseif(WINDOWS)
	set(_WIN_BACKEND "SDL")
elseif(ANDROID)
	set(_WIN_BACKEND "ANDROID")
elseif(MACOS)
	set(_WIN_BACKEND "SDL")
else()
	message(FATAL_ERROR "Couldn't determine the window backend. You need to specify it manually")
endif()

set(ANKI_WINDOW_BACKEND "${_WIN_BACKEND}" CACHE STRING "The window backend (GLXX11 or EGLX11 or EGLFBDEV or ANDROID or SDL or DUMMY)")

# Extra directories
set(ANKI_EXTRA_INCLUDE_DIRS CACHE STRING "Some extra include paths (Needed for some weird builds)")
set(ANKI_EXTRA_LIB_DIRS CACHE STRING "Some extra lib paths (Needed for some weird builds)")

# Valgrind
option(ANKI_VALGRIND_HAPPY "Make valgrind happy" OFF)

################################################################################
# Compiler & linker flags                                                      #
################################################################################

set(CXX_FLAGS "")
set(COMPILER_FLAGS "")
set(LINKER_FLAGS "")

# When building AnKi define this special flag
set(COMPILER_FLAGS "${COMPILER_FLAGS} -DANKI_BUILD ")

# address space
if(NOT ANKI_CPU_ADDR_SPACE STREQUAL "0")
	set(LINKER_FLAGS "${LINKER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
endif()

add_definitions(-fno-exceptions)

# static libstdc++
if(${CMAKE_C_COMPILER_ID} MATCHES "GNU")
	set(CXX_FLAGS "${CXX_FLAGS} -static-libstdc++ ")
endif()

# SSE
if(ANKI_ENABLE_SIMD)
	if(LINUX OR MACOS OR WINDOWS)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -msse4 ")
	else()
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -mfpu=neon ")
	endif()
endif()

# Add debug
if(ANKI_DEBUG_SYMBOLS)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -g3 ")
endif()

if(ANKI_DEBUG)
	if(${CMAKE_C_COMPILER_ID} MATCHES "GNU")
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fstack-check ")
	endif()
else()
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -DNODEBUG ")
	set(CXX_FLAGS "${CXX_FLAGS} -fno-rtti ")
endif()

# Add optimization flags
if(ANKI_OPTIMIZE)
	# -flto ?
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -ffast-math -O3 -flto ")

	# Add this because Android compiler complains
	if(ANDROID)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fno-data-sections ")
	endif()
else()
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -O0 ")
endif()

# Valgrind hacks
if(ANKI_VALGRIND_HAPPY)
	add_definitions("-DGLIBCXX_FORCE_NEW")
endif()

# Disable GLU in GLEW
add_definitions(-DGLEW_NO_GLU)

# Strip
if(ANKI_STRIP)
	set(LINKER_FLAGS "${LINKER_FLAGS} -s ")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -s ")
endif()

# gperftools
if(ANKI_WITH_GPERFTOOLS_PROF)
	LINK_DIRECTORIES("/home/godlike/src/more/gperftools/install/lib")
	set(ANKI_GPERFTOOLS_LIBS "profiler")
else()
	set(ANKI_GPERFTOOLS_LIBS "")
endif()

include_directories(${ANKI_EXTRA_INCLUDE_DIRS})
link_directories(${ANKI_EXTRA_LIB_DIRS})

# Set the flags to cmake now
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

################################################################################
# Install                                                                      #
################################################################################
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/" CACHE PATH "The subdirectory to the header prefix")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library install path")

message("++ Include install dir: ${INCLUDE_INSTALL_DIR}")
message("++ Lib install dir: ${LIB_INSTALL_DIR}")

################################################################################
# Thirdparty                                                                   #
################################################################################
set(ANKI_EXTERN_SUB_DIRS tinyxml2 lua z newton)

if(LINUX OR MACOS OR WINDOWS)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} GLEW)
endif()

if(ANKI_BUILD_TOOLS)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} assimp)
endif()

if(${ANKI_WINDOW_BACKEND} STREQUAL "SDL")
	include(ExternalProject)

	ExternalProject_Add(
		SDL2_PROJECT
		DOWNLOAD_COMMAND ""
		PREFIX "${CMAKE_CURRENT_BINARY_DIR}/sdl2_build"
		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2"
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/sdl2_build -DSDL_SHARED=OFF -DSDL_STATIC=ON -DCMAKE_BUILD_TYPE=Release)

	ExternalProject_Get_Property(SDL2_PROJECT install_dir)
	set(SDL2_INSTALL_DIR ${install_dir})

	add_library(SDL2_LIB STATIC IMPORTED)
	set_property(TARGET SDL2_LIB PROPERTY IMPORTED_LOCATION ${SDL2_INSTALL_DIR}/lib/libSDL2.a)
	add_dependencies(SDL2_LIB SDL2_PROJECT)
else()
	set(SDL2_INSTALL_DIR "")
endif()

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

	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

# Config.h
set(ANKI_VERSION_MAJOR 0)
set(ANKI_VERSION_MINOR 1)
message("++ AnKi version: ${ANKI_VERSION_MAJOR}.${ANKI_VERSION_MINOR}")

if(ANKI_DEBUG)
	set(ANKI_DEBUG 1)
else()
	set(ANKI_DEBUG 0)
endif()

CONFIGURE_FILE("include/anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h" DESTINATION "${INCLUDE_INSTALL_DIR}/anki")

# Include & lib directories
include_directories("thirdparty/tinyxml2/include" "thirdparty/lua" "thirdparty/z" "thirdparty/bullet"
	"${SDL2_INSTALL_DIR}/include/SDL2/" "include" "thirdparty/newton/newton" "${CMAKE_CURRENT_BINARY_DIR}")

if(LINUX OR MACOS OR WINDOWS)
	include_directories("thirdparty/GLEW/include")
else()
	#include_directories("thirdparty/GLES3/include")
endif()

if(ANDROID)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
endif()

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# AnKi compiler flags (Mainly warnings)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -W -Wextra -Wstrict-aliasing -Wwrite-strings -Wunused -Wunused-variable -Wno-unused-parameter -Wundef -std=c++11 ")

# Set platform specific
if(LINUX)
	set(_SYS GL ankiglew pthread dl)
elseif(MACOS)
	find_package(OpenGL REQUIRED)
	set(_SYS ${OPENGL_LIBRARIES} ankiglew pthread)
elseif(ANDROID)
	set(_SYS GLESv3 EGL log android)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
	set(_SYS_SRC "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
elseif(WINDOWS)
	set(_SYS ankiglew opengl32 version)

	add_definitions(-DGLEW_STATIC -D_NEWTON_STATIC_LIB)
else()
	message(FATAL_ERROR "Unhandled case")
endif()

if(${ANKI_WINDOW_BACKEND} STREQUAL "SDL")
	link_directories(${SDL2_INSTALL_DIR}/lib)
endif()

# Add anki sub libraries
set(ANKI_SUB_DIRS core script renderer scene ui event input physics resource misc gr collision math util)
set(ANKI_LIBS "")

foreach(TMP ${ANKI_SUB_DIRS})
	add_subdirectory(src/${TMP})
	set(ANKI_LIBS ${ANKI_LIBS} anki${TMP})
endforeach()

add_library(anki src/Dummy.cpp "${_SYS_SRC}")
add_dependencies(anki SDL2_LIB)
target_link_libraries(anki ${ANKI_LIBS} ankitinyxml2 ankilua ankiz ankinewton ${ANKI_GPERFTOOLS_LIBS} ${_SYS} SDL2_LIB ${FREETYPE_LIBRARIES})

################################################################################
# AnKi extra                                                                   #
################################################################################
if(ANKI_BUILD_TESTS)
	ENABLE_TESTING()
	add_subdirectory(tests)
endif()

if(ANKI_BUILD_TOOLS)
	add_subdirectory(tools)
endif()

if(ANKI_BUILD_TESTAPP)
	add_subdirectory(testapp)
endif()

if(ANKI_BUILD_BENCH)
	add_subdirectory(bench)
endif()

################################################################################
# Android                                                                      #
################################################################################
if(${ANDROID})
	# 1. generate Android.mk
	FILE(WRITE ./jni/Android.mk "APP_ABI := ${ANDROID_NDK_ABI_NAME}\n")

	# 2. generate gdb.setup
	GET_DIRECTORY_PROPERTY(include_directories DIRECTORY . include_directories)
	STRING(REGEX REPLACE ";" " " include_directories "${include_directories}")
	FILE(WRITE ./libs/${ANDROID_NDK_ABI_NAME}/gdb.setup "set solib-search-path ./libs/${ANDROID_NDK_ABI_NAME}\n")
	FILE(APPEND ./libs/${ANDROID_NDK_ABI_NAME}/gdb.setup "directory ${include_directories} ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}\n")

	# 3. copy gdbserver executable
	FILE(COPY ${ANDROID_NDK}/prebuilt/android-arm/gdbserver/gdbserver DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/)

	set(LIBRARY_NAME ankibench)

	# 4. copy lib to obj
	#ADD_CUSTOM_COMMAND(TARGET ${LIBRARY_NAME} POST_BUILD COMMAND mkdir -p ./obj/local/${ANDROID_NDK_ABI_NAME}/)
	#ADD_CUSTOM_COMMAND(TARGET ${LIBRARY_NAME} POST_BUILD COMMAND cp ./libs/${ANDROID_NDK_ABI_NAME}/lib${LIBRARY_NAME}.so ./obj/local/${ANDROID_NDK_ABI_NAME}/)
endif()
