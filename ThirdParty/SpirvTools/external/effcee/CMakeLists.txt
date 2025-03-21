cmake_minimum_required(VERSION 3.22.1)
project(effcee C CXX)
enable_testing()

# Require at least C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(${CMAKE_CXX_STANDARD} LESS 17)
	message(FATAL_ERROR "Effcee requires C++17 or later, but is configured for C++${CMAKE_CXX_STANDARD})")
endif()

option(EFFCEE_BUILD_TESTING "Enable testing for Effcee" ON)
if(${EFFCEE_BUILD_TESTING})
  message(STATUS "Configuring Effcee to build tests.")
  if(MSVC)
    # Our tests use ::testing::Combine.  Force the ability to use it, working
    # around googletest's possibly faulty compiler detection logic.
    # See https://github.com/google/googletest/issues/1352
    add_definitions(-DGTEST_HAS_COMBINE=1)
  endif(MSVC)
else()
  message(STATUS "Configuring Effcee to avoid building tests.")
endif()

option(EFFCEE_BUILD_SAMPLES "Enable building sample Effcee programs" ON)
if(${EFFCEE_BUILD_SAMPLES})
  message(STATUS "Configuring Effcee to build samples.")
else()
  message(STATUS "Configuring Effcee to avoid building samples.")
endif()

# RE2 needs Pthreads on non-WIN32
set(CMAKE_THREAD_LIBS_INIT "")
find_package(Threads)

include(cmake/setup_build.cmake)
include(cmake/utils.cmake)
include(GNUInstallDirs)

add_subdirectory(third_party)
add_subdirectory(effcee)
add_subdirectory(fuzzer)

if(${EFFCEE_BUILD_SAMPLES})
  add_subdirectory(examples)
endif()
