file(GLOB_RECURSE ANKI_GR_SOURCES *.cpp)
file(GLOB_RECURSE ANKI_GR_HEADERS *.h)

add_library(ankigr ${ANKI_GR_SOURCES} ${ANKI_GR_HEADERS})
target_link_libraries(ankigr ankiutil)
