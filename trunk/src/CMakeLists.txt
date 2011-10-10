PROJECT(ANKI_PROJ)

INCLUDE_DIRECTORIES(${ANKI_PROJ_SOURCE_DIR})

SET(ANKI_LIBS script renderer scene ui event 
	input physics resource core misc gl collision math util)

SUBDIRS(${ANKI_LIBS})

ADD_EXECUTABLE(anki Main.cpp)

TARGET_LINK_LIBRARIES(anki ${ANKI_LIBS} BulletSoftBody BulletDynamics 
	BulletCollision LinearMath GLEWmx GLU GL jpeg SDL png python2.6
	boost_system boost_python boost_filesystem boost_thread freetype) 