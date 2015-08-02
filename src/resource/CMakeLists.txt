file(GLOB_RECURSE ANKI_RSRC_SOURCES *.cpp)
file(GLOB_RECURSE ANKI_RSRC_HEADERS *.h)

add_library(ankiresource ${ANKI_RSRC_SOURCES} ${ANKI_RSRC_HEADERS})
target_link_libraries(ankiresource ankiphysics ankigr ankirenderer)
