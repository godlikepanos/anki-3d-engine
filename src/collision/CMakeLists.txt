file(GLOB ANKI_CLN_SOURCES *.cpp)
file(GLOB ANKI_CLN_HEADERS *.h)

add_library(ankicollision ${ANKI_CLN_SOURCES} ${ANKI_CLN_HEADERS})
target_link_libraries(ankicollision ankimath)
