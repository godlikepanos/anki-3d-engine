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

#include "anki/gl/ContextNonSharable.h"

#define ANKI_GL_CALL(func__) \
	do { \
		func__; \
	} while(0)

#endif
