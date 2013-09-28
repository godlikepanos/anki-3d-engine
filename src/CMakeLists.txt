#
# Add anki sub libraries
#
set(ANKI_SUB_DIRS script renderer scene event input physics resource core misc gl collision math util)

set(ANKI_LIBS "")

foreach(TMP ${ANKI_SUB_DIRS})
	add_subdirectory(${TMP})
	set(ANKI_LIBS ${ANKI_LIBS} anki${TMP})
endforeach()

# Set the platform libraries
if(LINUX)
	set(_SYS GL ankiglew pthread)
elseif(MACOS)
	find_package(OpenGL REQUIRED)
	set(_SYS ${OPENGL_LIBRARIES} ankiglew pthread)
elseif(ANDROID)
	set(_SYS GLESv3 EGL log android)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
	set(_SYS_SRC "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
else()
	message(FATAL_ERROR "Unhandled case")
endif()

if(${ANKI_WINDOW_BACKEND} STREQUAL "SDL")
	SET(ANKI_LIBS ${ANKI_LIBS} ankisdl)
endif()

ADD_LIBRARY(anki Dummy.cpp "${_SYS_SRC}")

TARGET_LINK_LIBRARIES(anki ${ANKI_LIBS} ankitinyxml2 ankilua ankibullet ankiz ${ANKI_GPERFTOOLS_LIBS} ${_SYS})

SET_TARGET_PROPERTIES(anki PROPERTIES LINKER_LANGUAGE CXX)

# Install
#
INSTALL(TARGETS anki DESTINATION ${LIB_INSTALL_DIR})

INSTALL(DIRECTORY ${ANKI_PROJECT_SOURCE_DIR}/anki DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h" PATTERN .svn EXCLUDE)
