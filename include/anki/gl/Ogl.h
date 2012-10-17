#ifndef ANKI_GL_OGL_H
#define ANKI_GL_OGL_H

#include "anki/Config.h"

#if ANKI_GL == ANKI_GL_DESKTOP
#	include <GL/glew.h>
#	if !defined(ANKI_GLEW_H)
#		error "Wrong GLEW included"
#	endif
#elif ANKI_GL == ANKI_GL_ES
#	include <GLES3/gl3.h>
#else
#	error "See file"
#endif

// The following macros are used for sanity checks in non sharable GL objects.
// They help us avoid binding those objects from other than the creation 
// threads. They are enabled only on debug
#if !ANKI_DEBUG
#	include <thread>

#	define ANKI_GL_NON_SHARABLE std::thread::id creationThreadId;

#	define ANKI_GL_NON_SHARABLE_INIT() \
		creationThreadId = std::this_thread::get_id()

#	define ANKI_GL_NON_SHARABLE_CHECK() \
		ANKI_ASSERT(creationThreadId == std::this_thread::get_id() \
			&& "Object can be used only from the creation thread")
#else
#	define ANKI_GL_NON_SHARABLE

#	define ANKI_GL_NON_SHARABLE_INIT() ((void)0)

#	define ANKI_GL_NON_SHARABLE_CHECK() ((void)0)
#endif

#endif
