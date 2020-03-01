file(GLOB_RECURSE TESTS_SOURCES *.cpp)
file(GLOB_RECURSE TESTS_HEADERS *.h)

include_directories("..")

add_executable(anki_tests ${TESTS_SOURCES} ${TESTS_HEADERS})
target_compile_definitions(anki_tests PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(anki_tests anki)

installExecutable(anki_tests)
install(TARGETS anki_tests DESTINATION "${CMAKE_INSTALL_PREFIX}/tests")
