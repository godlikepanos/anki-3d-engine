#include "anki/gl/GlStateMachine.h"


namespace anki {


//==============================================================================
// Statics                                                                     =
//==============================================================================

GLenum GlStateMachine::flagEnums[] =
{
	GL_DEPTH_TEST,
	GL_BLEND,
	GL_STENCIL_TEST,
	GL_CULL_FACE,
	GL_RASTERIZER_DISCARD,
	GL_POLYGON_OFFSET_FILL,
	0
};


//==============================================================================
// enable                                                                      =
//==============================================================================
void GlStateMachine::enable(GLenum glFlag, bool enable)
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
// isEnabled                                                                   =
//==============================================================================
bool GlStateMachine::isEnabled(GLenum glFlag)
{
	ANKI_ASSERT(flags.find(glFlag) != flags.end());
	bool state = flags[glFlag];
	ANKI_ASSERT(glIsEnabled(glFlag) == state);
	return state;
}


//==============================================================================
// sync                                                                        =
//==============================================================================
void GlStateMachine::sync()
{
	sProgGlId = getCurrentProgramGlId();

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
// setViewport                                                                 =
//==============================================================================
void GlStateMachine::setViewport(uint x, uint y, uint w, uint h)
{
	if(x != (uint)viewportX || y != (uint)viewportY ||
		w != (uint)viewportW || h != (uint)viewportH)
	{
		glViewport(x, y, w, h);
		viewportX = x;
		viewportY = y;
		viewportW = w;
		viewportH = h;
	}
}


//==============================================================================
// useShaderProg                                                               =
//==============================================================================
void GlStateMachine::useShaderProg(GLuint id)
{
	ANKI_ASSERT(getCurrentProgramGlId() == sProgGlId);

	if(sProgGlId != id)
	{
		glUseProgram(id);
		sProgGlId = id;
	}
}


//==============================================================================
// getCurrentProgramGlId                                                       =
//==============================================================================
GLuint GlStateMachine::getCurrentProgramGlId()
{
	int i;
	glGetIntegerv(GL_CURRENT_PROGRAM, &i);
	return i;
}


} // end namespace
