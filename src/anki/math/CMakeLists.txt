file(GLOB ANKI_M_SOURCES *.cpp)
file(GLOB ANKI_M_HEADERS *.h)

add_library(ankimath ${ANKI_M_SOURCES} ${ANKI_M_HEADERS})
