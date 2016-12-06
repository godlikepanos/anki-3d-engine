file(GLOB_RECURSE ANKI_SCENE_SOURCES *.cpp)
file(GLOB_RECURSE ANKI_SCENE_HEADERS *.h)

add_library(ankiscene ${ANKI_SCENE_SOURCES} ${ANKI_SCENE_HEADERS})

target_link_libraries(ankiscene ankievent)
