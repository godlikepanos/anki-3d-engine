if (EXISTS "${EFFCEE_FUZZED_DATA_PROVIDER_DIR}/FuzzedDataProvider.h")
  message(STATUS "effcee: configuring effcee-fuzz")
  add_executable(effcee-fuzz effcee_fuzz.cc)
  effcee_default_compile_options(effcee-fuzz)
  target_include_directories(effcee-fuzz PRIVATE "${EFFCEE_FUZZED_DATA_PROVIDER_DIR}")
  target_link_libraries(effcee-fuzz PRIVATE effcee)

  if(UNIX AND NOT MINGW)
    set_target_properties(effcee-fuzz PROPERTIES LINK_FLAGS -pthread)
  endif()
  if (WIN32 AND NOT MSVC)
    # For MinGW cross-compile, statically link to the C++ runtime
    set_target_properties(effcee-fuzz PROPERTIES
       LINK_FLAGS "-static -static-libgcc -static-libstdc++")
  endif(WIN32 AND NOT MSVC)
else()
  message(STATUS "effcee: effcee-fuzz won't be built.  Can't find FuzzedDataProvider.h")
endif()
