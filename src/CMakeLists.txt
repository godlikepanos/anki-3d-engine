#
# Set platform specific 
#
if(LINUX)
	set(_SYS GL ankiglew pthread)
elseif(MACOS)
	find_package(OpenGL REQUIRED)
	set(_SYS ${OPENGL_LIBRARIES} ankiglew pthread)
elseif(ANDROID)
	set(_SYS GLESv3 EGL log android)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
	set(_SYS_SRC "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")
elseif(WINDOWS)
	set(_SYS ankiglew opengl32 version)

	add_definitions("-DGLEW_STATIC")
else()
	message(FATAL_ERROR "Unhandled case")
endif()

#
# Add anki sub libraries
#
set(ANKI_SUB_DIRS core script renderer scene event input physics resource misc gr collision math util)

set(ANKI_LIBS "")

foreach(TMP ${ANKI_SUB_DIRS})
	add_subdirectory(${TMP})
endforeach()

if(${ANKI_WINDOW_BACKEND} STREQUAL "SDL")
	set(LIB_SDL ankisdl)
endif()

add_library(anki Dummy.cpp "${_SYS_SRC}")

target_link_libraries(anki ankicore ankiscript ankirenderer ankiscene ankievent ankiinput ankiphysics ankiresource ankimisc ankigr ankicollision ankimath ankiutil ankitinyxml2 ankilua ankiz ankinewton ${LIB_SDL} ${ANKI_GPERFTOOLS_LIBS} ${_SYS})

set_target_properties(anki PROPERTIES LINKER_LANGUAGE CXX)

# Install
#
install(TARGETS anki DESTINATION ${LIB_INSTALL_DIR})

install(DIRECTORY ${ANKI_PROJECT_SOURCE_DIR}/anki DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h" PATTERN .svn EXCLUDE)
