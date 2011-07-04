PROJECT(ANKI_PROJ)

INCLUDE_DIRECTORIES(${ANKI_PROJ_SOURCE_DIR})

SET(ANKI_DIRS Scripting Misc Renderer Scene Ui Events 
	Input Physics Resources GfxApi Collision Math Core Util)

SUBDIRS(${ANKI_DIRS})

ADD_EXECUTABLE(anki Main.cpp)

SET(ANKI_LIBS Scripting Core Renderer Scene Ui Events 
	Input Physics Resources Misc GfxApi Collision Math Core Util)

TARGET_LINK_LIBRARIES(anki ${ANKI_LIBS} BulletSoftBody BulletDynamics 
	BulletCollision LinearMath GLEW GLU GL jpeg SDL png python2.6
	boost_system boost_python boost_filesystem boost_thread freetype) 