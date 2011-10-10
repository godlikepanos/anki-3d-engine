SET(ANKI_LIBS script renderer scene ui event 
	input physics resource core misc gl collision math util)

SUBDIRS(${ANKI_LIBS})

ADD_LIBRARY(anki ${ANKI_LIBS})

TARGET_LINK_LIBRARIES(anki ${ANKI_LIBS} GLEWmx GLU GL jpeg SDL png python2.6
	boost_system boost_python boost_filesystem boost_thread freetype)

SET_TARGET_PROPERTIES(anki PROPERTIES LINKER_LANGUAGE CXX)
