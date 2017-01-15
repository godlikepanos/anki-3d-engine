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

if(${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
	set(GCC TRUE)
else()
	set(GCC FALSE)
endif()

################################################################################
# Some functions                                                               #
################################################################################

macro(defineStaticExternalLibrary extProj extLib extLibFname)
	add_library(${extLib} STATIC IMPORTED GLOBAL)
	set_property(TARGET ${extLib} PROPERTY IMPORTED_LOCATION ${extLibFname})
	add_dependencies(${extLib} ${extProj})
endmacro()

################################################################################
# Configuration                                                                #
################################################################################

option(ANKI_OPTIMIZE "Optimize" ON)
option(ANKI_DEBUG_SYMBOLS "Debug symbols" OFF)
option(ANKI_DEBUG "Debugging checks" OFF)
option(ANKI_LTO "LTO compilation" OFF)

option(ANKI_BUILD_TOOLS "Build tools" OFF)
option(ANKI_BUILD_TESTS "Build unit tests" OFF)
option(ANKI_BUILD_SANDBOX "Build sandbox application" ON)
option(ANKI_BUILD_BENCH "Build benchmark application" OFF)
option(ANKI_BUILD_SAMPLES "Build sample applications" ON)

option(ANKI_STRIP "Srip the symbols from the executables" OFF)

option(ANKI_ENABLE_TRACE "Enable performance tracing. Small overhead" OFF)
if(ANKI_ENABLE_TRACE)
	set(_ANKI_ENABLE_TRACE 1)
else()
	set(_ANKI_ENABLE_TRACE 0)
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

# Extra directories
set(ANKI_EXTRA_INCLUDE_DIRS CACHE STRING "Some extra include paths (Needed for some weird builds)")
set(ANKI_EXTRA_LIB_DIRS CACHE STRING "Some extra lib paths (Needed for some weird builds)")

# Valgrind
option(ANKI_VALGRIND_HAPPY "Make valgrind happy" OFF)

set(ANKI_GR_BACKEND "GL" CACHE STRING "The graphics API (GL or VULKAN)")

if(${ANKI_GR_BACKEND} STREQUAL "GL")
	set(GL TRUE)
	set(VULKAN FALSE)
else()
	set(GL FALSE)
	set(VULKAN TRUE)
endif()

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
if(GCC)
	#set(CXX_FLAGS "${CXX_FLAGS} -static-libstdc++ ")
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
	if(GCC)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fstack-check ")
	endif()
else()
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -DNODEBUG ")
	set(CXX_FLAGS "${CXX_FLAGS} -fno-rtti ")
endif()

# Add optimization flags
if(ANKI_OPTIMIZE)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -ffast-math -O3 ")

	# Add this because Android compiler complains
	if(ANDROID)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fno-data-sections ")
	endif()
else()
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -O0 ")
endif()

# LTO
if(ANKI_LTO)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -flto ")
	set(LINKER_FLAGS "${LINKER_FLAGS} -flto ")
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

include_directories(${ANKI_EXTRA_INCLUDE_DIRS})
link_directories(${ANKI_EXTRA_LIB_DIRS})

# Set the flags to cmake now
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

################################################################################
# Install                                                                      #
################################################################################
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/src/" CACHE PATH "The subdirectory to the header prefix")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library install path")

message("++ Include install dir: ${INCLUDE_INSTALL_DIR}")
message("++ Lib install dir: ${LIB_INSTALL_DIR}")

################################################################################
# Thirdparty                                                                   #
################################################################################
include(ExternalProject)

set(ANKI_EXTERN_SUB_DIRS tinyxml2 lua z)

if((LINUX OR MACOS OR WINDOWS) AND GL)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} GLEW)
endif()

if(VULKAN)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} SPIRV-Cross)
endif()

if(ANKI_BUILD_TOOLS)
	set(ANKI_EXTERN_SUB_DIRS ${ANKI_EXTERN_SUB_DIRS} assimp)
endif()

# SDL
if(SDL)
	ExternalProject_Add(
		SDL2_PROJECT
		DOWNLOAD_COMMAND ""
		PREFIX "${CMAKE_CURRENT_BINARY_DIR}/sdl2_build"
		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2"
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/sdl2_build -DSDL_SHARED=OFF -DSDL_STATIC=ON 
		-DCMAKE_BUILD_TYPE=Release -DDIRECTX=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DVIDEO_MIR=OFF 
		-DVIDEO_WAYLAND=OFF -DSNDIO=OFF -DVIDEO_OPENGLES=OFF)

	ExternalProject_Get_Property(SDL2_PROJECT install_dir)
	set(SDL2_INSTALL_DIR ${install_dir})

	defineStaticExternalLibrary(SDL2_PROJECT SDL2_LIB "${SDL2_INSTALL_DIR}/lib/libSDL2${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
	set(SDL2_INSTALL_DIR "")
endif()

# Newton
ExternalProject_Add(
	NEWTON_PROJECT
	DOWNLOAD_COMMAND ""
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/newton_build"
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/newton"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/newton_build -DCMAKE_BUILD_TYPE=Release 
	-DNEWTON_DEMOS_SANDBOX=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER})

defineStaticExternalLibrary(NEWTON_PROJECT NEWTON_LIB "${CMAKE_CURRENT_BINARY_DIR}/newton_build/src/NEWTON_PROJECT-build/lib/libNewton${CMAKE_STATIC_LIBRARY_SUFFIX}")
defineStaticExternalLibrary(NEWTON_PROJECT NEWTON_JOINTS_LIB "${CMAKE_CURRENT_BINARY_DIR}/newton_build/src/NEWTON_PROJECT-build/lib/libdCustomJoints${CMAKE_STATIC_LIBRARY_SUFFIX}")
defineStaticExternalLibrary(NEWTON_PROJECT NEWTON_CONTAINERS_LIB "${CMAKE_CURRENT_BINARY_DIR}/newton_build/src/NEWTON_PROJECT-build/lib/libdContainers${CMAKE_STATIC_LIBRARY_SUFFIX}")
defineStaticExternalLibrary(NEWTON_PROJECT NEWTON_MATH_LIB "${CMAKE_CURRENT_BINARY_DIR}/newton_build/src/NEWTON_PROJECT-build/lib/libdMath${CMAKE_STATIC_LIBRARY_SUFFIX}")

# freetype
ExternalProject_Add(
	FREETYPE_PROJECT
	DOWNLOAD_COMMAND ""
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/freetype_build"
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/freetype"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/freetype_build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER})

ExternalProject_Get_Property(FREETYPE_PROJECT install_dir)
set(FREETYPE_INSTALL_DIR ${install_dir})
defineStaticExternalLibrary(FREETYPE_PROJECT FREETYPE_LIB "${FREETYPE_INSTALL_DIR}/lib/libfreetype${CMAKE_STATIC_LIBRARY_SUFFIX}")

# glslang
if(VULKAN)
	ExternalProject_Add(
		GLSLANG_PROJECT
		DOWNLOAD_COMMAND ""
		PREFIX "${CMAKE_CURRENT_BINARY_DIR}/glslang_build"
		SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glslang"
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/glslang_build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER})

	ExternalProject_Get_Property(GLSLANG_PROJECT install_dir)
	set(GLSLANG_INSTALL_DIR ${install_dir})

	defineStaticExternalLibrary(GLSLANG_PROJECT GLSLANG_LIB "${GLSLANG_INSTALL_DIR}/lib/libglslang${CMAKE_STATIC_LIBRARY_SUFFIX}")
	defineStaticExternalLibrary(GLSLANG_PROJECT SPIRV_LIB "${GLSLANG_INSTALL_DIR}/lib/libSPIRV${CMAKE_STATIC_LIBRARY_SUFFIX}")
	defineStaticExternalLibrary(GLSLANG_PROJECT OSD_LIB "${GLSLANG_INSTALL_DIR}/lib/libOSDependent${CMAKE_STATIC_LIBRARY_SUFFIX}")
	defineStaticExternalLibrary(GLSLANG_PROJECT GLC_LIB "${GLSLANG_INSTALL_DIR}/lib/libOGLCompiler${CMAKE_STATIC_LIBRARY_SUFFIX}")
	defineStaticExternalLibrary(GLSLANG_PROJECT HLSL_LIB "${GLSLANG_INSTALL_DIR}/lib/libHLSL${CMAKE_STATIC_LIBRARY_SUFFIX}")
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

	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile 
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
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

configure_file("src/anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h" DESTINATION "${INCLUDE_INSTALL_DIR}/anki")

# Include & lib directories
include_directories("src" 
	"thirdparty/tinyxml2/include" 
	"thirdparty/lua" 
	"thirdparty/z" 
	"${SDL2_INSTALL_DIR}/include/SDL2/" 
	"${FREETYPE_INSTALL_DIR}/include/freetype2/" 
	"thirdparty/newton/coreLibrary_300/source/newton" 
	"thirdparty/newton/packages/dCustomJoints"
	"thirdparty/newton/packages/dContainers" 
	"thirdparty/newton/packages/dMath" 
	"thirdparty/newton/packages/thirdParty/timeTracker/"
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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -W -Wextra -Wstrict-aliasing -Wwrite-strings -Wunused -Wunused-variable -Wno-unused-parameter -Wundef -Wno-ignored-attributes -std=c++11 ")

# Set platform specific
if(LINUX)
	if(GL)
		set(_SYS ${ANKI_GR_BACKEND} ankiglew)
	else()
		set(_SYS vulkan)
		if(SDL)
			set(_SYS ${_SYS} X11-xcb)
		else()
			message(FATAL_ERROR "Unhandled case")
		endif()
	endif()

	set(_SYS ${_SYS} pthread dl)
elseif(MACOS)
	find_package(OpenGL REQUIRED)
	set(_SYS ${OPENGL_LIBRARIES} ankiglew pthread)
elseif(ANDROID)
	set(_SYS GLESv3 EGL log android)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
	set(_SYS_SRC "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
elseif(WINDOWS)
	if(GL)
		set(_SYS ankiglew opengl32)
	else()
		set(_SYS vulkan-1)
	endif()

	set(_SYS ${_SYS} version Imm32 Winmm)
	add_definitions(-DGLEW_STATIC -D_NEWTON_STATIC_LIB)
else()
	message(FATAL_ERROR "Unhandled case")
endif()

if(SDL)
	link_directories(${SDL2_INSTALL_DIR}/lib)
endif()

link_directories(${FREETYPE_INSTALL_DIR}/lib)

add_definitions(-DSPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)

# Add anki sub libraries
set(ANKI_SUB_DIRS core script renderer scene ui event input physics resource misc gr collision math util)
set(ANKI_LIBS "")

foreach(TMP ${ANKI_SUB_DIRS})
	add_subdirectory(src/anki/${TMP})
	set(ANKI_LIBS ${ANKI_LIBS} anki${TMP})
endforeach()

add_library(anki src/anki/Dummy.cpp "${_SYS_SRC}")

target_link_libraries(anki ${ANKI_LIBS} ankitinyxml2 ankilua ankiz FREETYPE_LIB ${_SYS})

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

if(ANKI_BUILD_SANDBOX)
	add_subdirectory(sandbox)
endif()

if(ANKI_BUILD_SAMPLES)
	add_subdirectory(samples)
endif()

if(ANKI_BUILD_BENCH)
	add_subdirectory(bench)
endif()
