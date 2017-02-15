file(GLOB ANKI_PHYS_SOURCES *.cpp)
file(GLOB ANKI_PHYS_HEADERS *.h)

add_library(ankiphysics ${ANKI_PHYS_SOURCES} ${ANKI_PHYS_HEADERS})
target_link_libraries(ankiphysics ankinewton)

