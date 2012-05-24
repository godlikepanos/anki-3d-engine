#include "anki/gl/GlState.h"
#include "anki/util/Assert.h"

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
void GlState::setViewport(uint x, uint y, uint w, uint h)
{
	if(x != (uint)viewportX || y != (uint)viewportY 
		|| w != (uint)viewportW || h != (uint)viewportH)
	{
		glViewport(x, y, w, h);
		viewportX = x;
		viewportY = y;
		viewportW = w;
		viewportH = h;
	}
}

} // end namespace
