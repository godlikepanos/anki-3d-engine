include(CheckCCompilerFlag)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

# Developer options

option(REPROC_DEVELOP "Enable all developer options" $ENV{REPROC_DEVELOP})
option(REPROC_TEST "Build tests" ${REPROC_DEVELOP})
option(REPROC_EXAMPLES "Build examples" ${REPROC_DEVELOP})
option(REPROC_WARNINGS "Enable compiler warnings" ${REPROC_DEVELOP})
option(REPROC_TIDY "Run clang-tidy when building" ${REPROC_DEVELOP})

option(
  REPROC_SANITIZERS
  "Build with sanitizers on configurations that support it"
  ${REPROC_DEVELOP}
)

option(
  REPROC_WARNINGS_AS_ERRORS
  "Add -Werror or equivalent to the compile flags and clang-tidy"
)

mark_as_advanced(
  REPROC_TIDY
  REPROC_SANITIZERS
  REPROC_WARNINGS_AS_ERRORS
)

# Installation options

option(REPROC_OBJECT_LIBRARIES "Build CMake object libraries" ${REPROC_DEVELOP})

if(NOT REPROC_OBJECT_LIBRARIES)
  set(REPROC_INSTALL_DEFAULT ON)
endif()

option(REPROC_INSTALL "Generate installation rules" ${REPROC_INSTALL_DEFAULT})
option(REPROC_INSTALL_PKGCONFIG "Install pkg-config files" ON)

set(
  REPROC_INSTALL_CMAKECONFIGDIR
  ${CMAKE_INSTALL_LIBDIR}/cmake
  CACHE STRING "CMake config files installation directory"
)

set(
  REPROC_INSTALL_PKGCONFIGDIR
  ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  CACHE STRING "pkg-config files installation directory"
)

mark_as_advanced(
  REPROC_OBJECT_LIBRARIES
  REPROC_INSTALL
  REPROC_INSTALL_PKGCONFIG
  REPROC_INSTALL_CMAKECONFIGDIR
  REPROC_INSTALL_PKGCONFIGDIR
)

# Testing

if(REPROC_TEST)
  enable_testing()
endif()

# Build type

if(REPROC_DEVELOP AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo
  )
endif()

# clang-tidy

if(REPROC_TIDY)
  find_program(REPROC_TIDY_PROGRAM clang-tidy)
  mark_as_advanced(REPROC_TIDY_PROGRAM)

  if(NOT REPROC_TIDY_PROGRAM)
    message(FATAL_ERROR "clang-tidy not found")
  endif()

  if(REPROC_WARNINGS_AS_ERRORS)
    set(REPROC_TIDY_PROGRAM ${REPROC_TIDY_PROGRAM} -warnings-as-errors=*)
  endif()
endif()

# Functions

function(reproc_common TARGET LANGUAGE NAME DIRECTORY)
  if(LANGUAGE STREQUAL C)
    set(STANDARD 99)
    target_compile_features(${TARGET} PUBLIC c_std_99)
  else()
    # clang-tidy uses the MSVC standard library instead of MinGW's standard
    # library so we have to use C++14 (because MSVC headers use C++14).
    if(MINGW AND REPROC_TIDY)
      set(STANDARD 14)
    else()
      set(STANDARD 11)
    endif()

    target_compile_features(${TARGET} PUBLIC cxx_std_11)
  endif()

  set_target_properties(${TARGET} PROPERTIES
    ${LANGUAGE}_STANDARD ${STANDARD}
    ${LANGUAGE}_STANDARD_REQUIRED ON
    ${LANGUAGE}_EXTENSIONS OFF
    OUTPUT_NAME "${NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${DIRECTORY}"
  )

  if(REPROC_TIDY AND REPROC_TIDY_PROGRAM)
    set_property(
      TARGET ${TARGET}
      # `REPROC_TIDY_PROGRAM` is a list so we surround it with quotes to pass it
      # as a single argument.
      PROPERTY ${LANGUAGE}_CLANG_TIDY "${REPROC_TIDY_PROGRAM}"
    )
  endif()

  # Common development flags (warnings + sanitizers + colors)

  if(REPROC_WARNINGS)
    if(MSVC)
      check_c_compiler_flag(/permissive- REPROC_HAVE_PERMISSIVE)

      target_compile_options(${TARGET} PRIVATE
        /nologo # Silence MSVC compiler version output.
        $<$<BOOL:${REPROC_WARNINGS_AS_ERRORS}>:/WX> # -Werror
        $<$<BOOL:${REPROC_HAVE_PERMISSIVE}>:/permissive->
      )

      if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.15.0)
        # CMake 3.15 does not add /W3 to the compiler flags by default anymore
        # so we add /W4 instead.
        target_compile_options(${TARGET} PRIVATE /W4)
      endif()

      if(LANGUAGE STREQUAL C)
        # Disable MSVC warnings that flag C99 features as non-standard.
        target_compile_options(${TARGET} PRIVATE /wd4204 /wd4221)
      endif()
    else()
      target_compile_options(${TARGET} PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Wconversion
        -Wsign-conversion
        $<$<BOOL:${REPROC_WARNINGS_AS_ERRORS}>:-Werror>
        $<$<BOOL:${REPROC_WARNINGS_AS_ERRORS}>:-pedantic-errors>
      )

      if(LANGUAGE STREQUAL C OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
        target_compile_options(${TARGET} PRIVATE -Wmissing-prototypes)
      endif()
    endif()

    if(WIN32)
      target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    target_compile_options(${TARGET} PRIVATE
      $<$<${LANGUAGE}_COMPILER_ID:GNU>:-fdiagnostics-color>
      $<$<${LANGUAGE}_COMPILER_ID:Clang>:-fcolor-diagnostics>
    )
  endif()

  if(REPROC_SANITIZERS AND NOT MSVC AND NOT MINGW)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.15.0)
      set_property(
        TARGET ${TARGET}
        PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded
      )
    endif()

    target_compile_options(${TARGET} PRIVATE
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
    )

    target_link_libraries(${TARGET} PRIVATE -fsanitize=address,undefined)
  endif()
endfunction()

function(reproc_library TARGET LANGUAGE)
  if(REPROC_OBJECT_LIBRARIES)
    add_library(${TARGET} OBJECT)
  else()
    add_library(${TARGET})
  endif()

  reproc_common(${TARGET} ${LANGUAGE} "" lib)

  if(BUILD_SHARED_LIBS AND NOT REPROC_OBJECT_LIBRARIES)
    # Enable -fvisibility=hidden and -fvisibility-inlines-hidden.
    set_target_properties(${TARGET} PROPERTIES
      ${LANGUAGE}_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN true
    )

    # clang-tidy errors with: unknown argument: '-fno-keep-inline-dllexport'
    # when enabling `VISIBILITY_INLINES_HIDDEN` on MinGW so we disable it when
    # running clang-tidy on MinGW.
    if(MINGW AND REPROC_TIDY)
      set_property(TARGET ${TARGET} PROPERTY VISIBILITY_INLINES_HIDDEN false)
    endif()

    # Disable CMake's default export definition.
    set_property(TARGET ${TARGET} PROPERTY DEFINE_SYMBOL "")

    string(TOUPPER ${TARGET} TARGET_UPPER)
    string(REPLACE + X TARGET_SANITIZED ${TARGET_UPPER})

    target_compile_definitions(${TARGET} PRIVATE ${TARGET_SANITIZED}_BUILDING)
    if(WIN32)
      target_compile_definitions(${TARGET} PUBLIC ${TARGET_SANITIZED}_SHARED)
    endif()
  endif()

  # Make sure we follow the popular naming convention for shared libraries on
  # UNIX systems.
  set_target_properties(${TARGET} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

  # Only use the headers from the repository when building. When installing we
  # want to use the install location of the headers (e.g. /usr/include) as the
  # include directory instead.
  target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )

  # Adapted from https://codingnest.com/basic-cmake-part-2/.
  # Each library is installed separately (with separate config files).

  if(REPROC_INSTALL)

    # Headers

    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      COMPONENT ${TARGET}-development
    )

    # Library

    install(
      TARGETS ${TARGET}
      EXPORT ${TARGET}-targets
      RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${TARGET}-runtime
      LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT ${TARGET}-runtime
        NAMELINK_COMPONENT ${TARGET}-development
      ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT ${TARGET}-development
      INCLUDES
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    if(NOT APPLE)
      set_property(
        TARGET ${TARGET}
        PROPERTY INSTALL_RPATH $ORIGIN
      )
    endif()

    # CMake config

    configure_package_config_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}-config.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config.cmake
      INSTALL_DESTINATION ${REPROC_INSTALL_CMAKECONFIGDIR}/${TARGET}
    )

    write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config-version.cmake
      COMPATIBILITY SameMajorVersion
    )

    install(
      FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}-config-version.cmake
      DESTINATION ${REPROC_INSTALL_CMAKECONFIGDIR}/${TARGET}
      COMPONENT ${TARGET}-development
    )

    install(
      EXPORT ${TARGET}-targets
      DESTINATION ${REPROC_INSTALL_CMAKECONFIGDIR}/${TARGET}
      COMPONENT ${TARGET}-development
    )

    # pkg-config

    if(REPROC_INSTALL_PKGCONFIG)
      configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.pc
        @ONLY
      )

      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.pc
        DESTINATION ${REPROC_INSTALL_PKGCONFIGDIR}
        COMPONENT ${TARGET}-development
      )
    endif()
  endif()
endfunction()

function(reproc_test TARGET NAME LANGUAGE)
  if(NOT REPROC_TEST)
    return()
  endif()

  if(LANGUAGE STREQUAL C)
    set(EXTENSION c)
  else()
    set(EXTENSION cpp)
  endif()

  add_executable(${TARGET}-test-${NAME} test/${NAME}.${EXTENSION})

  reproc_common(${TARGET}-test-${NAME} ${LANGUAGE} ${NAME} test)
  target_link_libraries(${TARGET}-test-${NAME} PRIVATE ${TARGET})

  if(MINGW)
    target_compile_definitions(${TARGET}-test-${NAME} PRIVATE
      __USE_MINGW_ANSI_STDIO=1 # Add %zu on Mingw
    )
  endif()

  add_test(NAME ${TARGET}-test-${NAME} COMMAND ${TARGET}-test-${NAME})

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/${NAME}.c)
    target_compile_definitions(${TARGET}-test-${NAME} PRIVATE
      RESOURCE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/resources"
    )

    if (NOT TARGET ${TARGET}-resource-${NAME})
      add_executable(${TARGET}-resource-${NAME} resources/${NAME}.c)
      reproc_common(${TARGET}-resource-${NAME} C ${NAME} resources)
    endif()

    # Make sure the test resource is available when running the test.
    add_dependencies(${TARGET}-test-${NAME} ${TARGET}-resource-${NAME})
  endif()
endfunction()

function(reproc_example TARGET NAME LANGUAGE)
  cmake_parse_arguments(OPT "" "" "ARGS;DEPENDS" ${ARGN})
  if(NOT REPROC_EXAMPLES)
    return()
  endif()

  if(LANGUAGE STREQUAL C)
    set(EXTENSION c)
  else()
    set(EXTENSION cpp)
  endif()

  add_executable(${TARGET}-example-${NAME} examples/${NAME}.${EXTENSION})

  reproc_common(${TARGET}-example-${NAME} ${LANGUAGE} ${NAME} examples)
  target_link_libraries(${TARGET}-example-${NAME} PRIVATE ${TARGET} ${OPT_DEPENDS})

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/${NAME}.c)
    target_compile_definitions(${TARGET}-example-${NAME} PRIVATE
      RESOURCE_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/resources"
    )

    if (NOT TARGET ${TARGET}-resource-${NAME})
      add_executable(${TARGET}-resource-${NAME} resources/${NAME}.c)
      reproc_common(${TARGET}-resource-${NAME} C ${NAME} resources)
    endif()

    # Make sure the example resource is available when running the example.
    add_dependencies(${TARGET}-example-${NAME} ${TARGET}-resource-${NAME})
  endif()

  if(REPROC_TEST)
    if(NOT DEFINED OPT_ARGS)
      set(OPT_ARGS cmake --help)
    endif()

    add_test(
      NAME ${TARGET}-example-${NAME}
      COMMAND ${TARGET}-example-${NAME} ${OPT_ARGS}
    )
  endif()
endfunction()
