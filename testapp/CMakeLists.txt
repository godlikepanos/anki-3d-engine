SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors -pedantic -ansi -Wall -W -Wwrite-strings -Wno-unused -Werror -Wno-long-long -msse4 -std=c++11 ")

ADD_DEFINITIONS("-Dthread_local=__thread")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../extern/GLEW/include ${CMAKE_CURRENT_SOURCE_DIR}/../extern/tinyxml2/include ${CMAKE_CURRENT_SOURCE_DIR}/../extern/lua)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../extern/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../extern/lib64)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADD_EXECUTABLE(testapp Main.cpp)

TARGET_LINK_LIBRARIES(testapp anki) 
