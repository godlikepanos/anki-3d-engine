file(GLOB_RECURSE ANKI_GL_SOURCES *.cpp)
file(GLOB_RECURSE ANKI_GL_HEADERS *.h)

add_library(ankigl ${ANKI_GL_SOURCES} ${ANKI_GL_HEADERS})
target_link_libraries(ankigl ankiutil)
