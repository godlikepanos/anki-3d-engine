#
# Add anki sub libraries
#
SET(ANKI_SUB_DIRS script renderer scene event input physics resource core misc gl collision math util)

SET(ANKI_LIBS "")

FOREACH(TMP ${ANKI_SUB_DIRS})
	ADD_SUBDIRECTORY(${TMP})
	SET(ANKI_LIBS ${ANKI_LIBS} anki${TMP})
ENDFOREACH()

# Set the platform libraries
IF(ANKI_WINDOW_BACKEND STREQUAL "GLXX11")
	SET(_SYS GL ankiglew)
ELSEIF(ANKI_WINDOW_BACKEND STREQUAL "MACOS")
	FIND_PACKAGE(OpenGL REQUIRED)
	SET(_SYS ${OPENGL_LIBRARIES} ankiglew)
ELSEIF(ANKI_WINDOW_BACKEND STREQUAL "ANDROID")
	SET(_SYS GLESv2 EGL log android)
	INCLUDE_DIRECTORIES("${ANDROID_NDK}/sources/android/native_app_glue")
	SET(_SYS_SRC "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
ELSE()
	SET(_SYS GLESv2 EGL mali)
ENDIF()

ADD_LIBRARY(anki Dummy.cpp "${_SYS_SRC}")

TARGET_LINK_LIBRARIES(anki ${ANKI_LIBS} ankitinyxml2 ankilua ankibullet ankiz ankipng ${ANKI_GPERFTOOLS_LIBS} pthread ${_SYS})

SET_TARGET_PROPERTIES(anki PROPERTIES LINKER_LANGUAGE CXX)

# Install
#
INSTALL(TARGETS anki DESTINATION ${LIB_INSTALL_DIR})

INSTALL(DIRECTORY ${ANKI_PROJECT_SOURCE_DIR}/anki DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h" PATTERN .svn EXCLUDE)
