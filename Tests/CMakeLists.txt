file(GLOB_RECURSE TESTS_SOURCES *.cpp)
file(GLOB_RECURSE TESTS_HEADERS *.h)

include_directories("..")

add_executable(Tests ${TESTS_SOURCES} ${TESTS_HEADERS})
target_compile_definitions(Tests PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(Tests anki)

installExecutable(Tests)
