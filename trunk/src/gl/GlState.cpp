#include "anki/gl/GlState.h"
#include "anki/gl/ShaderProgram.h"
#include "anki/gl/Fbo.h"
#include "anki/gl/Vao.h"
#include "anki/util/Assert.h"
#include <limits>

#if ANKI_DEBUG
#	define CHECK_GL() ANKI_ASSERT(initialized == true)
#else
#	define CHECK_GL() ((void)0)
#endif

namespace anki {

//==============================================================================

GLenum GlState::flagEnums[] = {
	GL_DEPTH_TEST,
	GL_BLEND,
	GL_STENCIL_TEST,
	GL_CULL_FACE,
	GL_RASTERIZER_DISCARD,
	GL_POLYGON_OFFSET_FILL,
	0
};

//==============================================================================
void GlState::enable(GLenum glFlag, bool enable)
{
	CHECK_GL();
	ANKI_ASSERT(flags.find(glFlag) != flags.end());
	bool state = flags[glFlag];
	ANKI_ASSERT(glIsEnabled(glFlag) == state);

	if(enable != state)
	{
		if(enable)
		{
			glEnable(glFlag);
		}
		else
		{
			glDisable(glFlag);
		}
		flags[glFlag] = enable;
	}
}

//==============================================================================
bool GlState::isEnabled(GLenum glFlag)
{
	CHECK_GL();
	ANKI_ASSERT(flags.find(glFlag) != flags.end());
	bool state = flags[glFlag];
	ANKI_ASSERT(glIsEnabled(glFlag) == state);
	return state;
}

//==============================================================================
void GlState::sync()
{
	// Set flags
	GLenum* flagEnum = &flagEnums[0];
	while(*flagEnum != 0)
	{
		flags[*flagEnum] = glIsEnabled(*flagEnum);
		++flagEnum;
	}

	// viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, &viewport[0]);
	viewportX = viewport[0];
	viewportY = viewport[1];
	viewportW = viewport[2];
	viewportH = viewport[3];
}

//==============================================================================
void GlState::setViewport(U32 x, U32 y, U32 w, U32 h)
{
	CHECK_GL();
	if(x != (U32)viewportX || y != (U32)viewportY 
		|| w != (U32)viewportW || h != (U32)viewportH)
	{
		glViewport(x, y, w, h);
		viewportX = x;
		viewportY = y;
		viewportW = w;
		viewportH = h;
	}
}

//==============================================================================
void GlState::setProgram(ShaderProgram* prog_)
{
	CHECK_GL();
	ANKI_ASSERT(prog_);
	prog_->bind();
}

//==============================================================================
void GlState::setFbo(Fbo* fbo_)
{
	CHECK_GL();
	if(fbo_ == nullptr)
	{
		Fbo::unbindAll();
	}
	else
	{
		ANKI_ASSERT(fbo_->isComplete());
		fbo_->bind();
	}
}

//==============================================================================
void GlState::setVao(Vao* vao_)
{
	CHECK_GL();
	ANKI_ASSERT(vao_);
	vao_->bind();
}

} // end namespace
