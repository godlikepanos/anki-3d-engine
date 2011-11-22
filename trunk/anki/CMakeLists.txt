SET(ANKI_LIBS script renderer scene ui event 
	input physics resource core misc gl collision math util)

SUBDIRS(${ANKI_LIBS})

ADD_LIBRARY(anki ${ANKI_LIBS})

TARGET_LINK_LIBRARIES(anki ${ANKI_LIBS} BulletSoftBody BulletDynamics 
	BulletCollision LinearMath GLEWmx GLU GL jpeg SDL png python2.6
	boost_system boost_python boost_filesystem boost_thread freetype)

SET_TARGET_PROPERTIES(anki PROPERTIES LINKER_LANGUAGE CXX)

INSTALL(TARGETS anki DESTINATION ${LIB_INSTALL_DIR})

INSTALL(DIRECTORY ${ANKI_PROJECT_SOURCE_DIR}/anki DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h" PATTERN .svn EXCLUDE)
