#ifndef ANKI_GL_OGL_H
#define ANKI_GL_OGL_H

#include <GL/glew.h>
#if !defined(ANKI_GLEW_H)
#	error "Wrong GLEW included"
#endif

// The following macros are used for sanity checks in non sharable GL objects.
// They help us avoid binding those objects from other than the creation 
// threads. They are enabled only on debug
#if !NDEBUG
#	include <thread>

#	define ANKI_GL_NON_SHARABLE std::thread::id creationThreadId;

#	define ANKI_GL_NON_SHARABLE_INIT() \
		creationThreadId = std::this_thread::get_id()

#	define ANKI_GL_NON_SHARABLE_CHECK() \
		ANKI_ASSERT(creationThreadId == std::this_thread::get_id() \
			&& "FBO can be used only from the creation thread")
#else
#	define ANKI_GL_NON_SHARABLE

#	define ANKI_GL_NON_SHARABLE_INIT() (void)

#	define ANKI_GL_NON_SHARABLE_CHECK() (void)
#endif

#endif
