file(GLOB ANKI_UI_SOURCES *.cpp)

add_library(ankiui ${ANKI_UI_SOURCES})
target_link_libraries(ankiui ankiresource freetype)
