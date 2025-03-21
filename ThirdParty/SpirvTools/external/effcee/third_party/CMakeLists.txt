# Suppress all warnings from third-party projects.
set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS -w)

# Set alternate root directory for third party sources.
set(EFFCEE_THIRD_PARTY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING
  "Root location of all third_party projects")

# Find googletest and gmock
if(${googletest-distribution_SOURCE_DIR})
  set(EFFCEE_GOOGLETEST_DIR "${googletest-distribution_SOURCE_DIR}" CACHE STRING
	  "Location of googletest source")
else()
  set(EFFCEE_GOOGLETEST_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/googletest" CACHE STRING
          "Location of googletest source")
endif()

# Find abseil
if(absl_SOURCE_DIR)
  set(EFFCEE_ABSEIL_DIR "${absl_SOURCE_DIR}" CACHE STRING "Location of abseil source" FORCE)
else()
  # Allow for abseil-cpp or abseil_cpp
  if (IS_DIRECTORY "${EFFCEE_THIRD_PARTY_ROOT_DIR}/abseil_cpp")
    set(EFFCEE_ABSEIL_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/abseil_cpp" CACHE STRING "Location of abseil source")
  elseif(IS_DIRECTORY "${EFFCEE_THIRD_PARTY_ROOT_DIR}/abseil-cpp")
    set(EFFCEE_ABSEIL_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/abseil-cpp" CACHE STRING "Location of abseil source")
  endif()
endif()

# Find re2
if(RE2_SOURCE_DIR)
  set(EFFCEE_RE2_DIR "${RE2_SOURCE_DIR}" CACHE STRING "Location of re2 source" FORCE)
else()
  set(EFFCEE_RE2_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/re2" CACHE STRING
    "Location of re2 source")
endif()

# Configure third party projects.
if(EFFCEE_BUILD_TESTING)
  if (NOT TARGET gmock)
    if (IS_DIRECTORY ${EFFCEE_GOOGLETEST_DIR})
      add_subdirectory(${EFFCEE_GOOGLETEST_DIR} googletest EXCLUDE_FROM_ALL)
    endif()
  endif()
  if (NOT TARGET gmock)
    message(FATAL_ERROR "gmock was not found - required for tests")
  endif()
endif()

if (NOT TARGET absl::base)
  if (IS_DIRECTORY ${EFFCEE_ABSEIL_DIR})
    set(ABSL_INTERNAL_AT_LEAST_CXX17 ON)
    set(ABSL_PROPAGATE_CXX_STD ON)
    set(ABSL_ENABLE_INSTALL ON)
    add_subdirectory(${EFFCEE_ABSEIL_DIR} absl EXCLUDE_FROM_ALL)
  endif()
endif()
if (NOT TARGET absl::base)
  message(FATAL_ERROR "absl was not found - required for compilation")
endif()

if (NOT TARGET re2)
  if (IS_DIRECTORY ${EFFCEE_RE2_DIR})
    add_subdirectory(${EFFCEE_RE2_DIR} re2 EXCLUDE_FROM_ALL)
  endif()
endif()
if (NOT TARGET re2)
  message(FATAL_ERROR "re2 was not found - required for compilation")
endif()
