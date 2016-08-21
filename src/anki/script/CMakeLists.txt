file(GLOB_RECURSE ANKI_SCRIPT_SOURCES *.cpp)
file(GLOB_RECURSE ANKI_SCRIPT_HEADERS *.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(ankiscript ${ANKI_SCRIPT_SOURCES} ${ANKI_SCRIPT_HEADERS})
target_link_libraries(ankiscript ankirenderer ankiscene ankimath)
