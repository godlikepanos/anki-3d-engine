file(GLOB_RECURSE ANKI_R_SOURCES *.cpp)
file(GLOB_RECURSE ANKI_R_HEADERS *.h)

add_library(ankirenderer ${ANKI_R_SOURCES} ${ANKI_R_HEADERS})
target_link_libraries(ankirenderer ankicore ankiui)
