#include "anki/gl/GlState.h"
#include "anki/gl/ShaderProgram.h"
#include "anki/gl/Fbo.h"
#include "anki/gl/Vao.h"
#include "anki/util/Assert.h"
#include <limits>
#include <algorithm>

namespace anki {

//==============================================================================
// GlStateCommon                                                               = 
//==============================================================================

//==============================================================================
void GlStateCommon::init(const U32 major_, const U32 minor_)
{
	major = (I32)major_;
	minor = (I32)minor_;

	std::string glstr = (const char*)glGetString(GL_VENDOR);
	glstr += (const char*)glGetString(GL_RENDERER);
	std::transform(glstr.begin(), glstr.end(), glstr.begin(), ::tolower);

	if(glstr.find("arm") != std::string::npos)
	{
		gpu = GPU_ARM;
	}
	else if(glstr.find("nvidia") != std::string::npos)
	{
		gpu = GPU_NVIDIA;
	}
}

//==============================================================================
// GlState                                                                     = 
//==============================================================================

//==============================================================================
static Array<GLenum, 7> glCaps = {{
	GL_DEPTH_TEST,
	GL_BLEND,
	GL_STENCIL_TEST,
	GL_CULL_FACE,
	GL_RASTERIZER_DISCARD,
	GL_POLYGON_OFFSET_FILL,
	GL_SCISSOR_TEST
}};

//==============================================================================
U GlState::getIndexFromGlEnum(const GLenum cap)
{
	static_assert(decltype(GlState::flags)::getSize() == 7, "See file");
	U out = false;
	switch(cap)
	{
	case GL_DEPTH_TEST:
		out = 0;
		break;
	case GL_BLEND:
		out = 1;
		break;
	case GL_STENCIL_TEST:
		out = 2;
		break;
	case GL_CULL_FACE:
		out = 3;
		break;
	case GL_RASTERIZER_DISCARD:
		out = 4;
		break;
	case GL_POLYGON_OFFSET_FILL:
		out = 5;
		break;
	case GL_SCISSOR_TEST:
		out = 6;
	default:
		ANKI_ASSERT(0);
		break;
	}
	return out;
}

//==============================================================================
void GlState::enable(GLenum cap, bool enable)
{
	U index = getIndexFromGlEnum(cap);
	Bool state = flags[index];
	ANKI_ASSERT(glIsEnabled(cap) == state);

	if(enable != state)
	{
		if(enable)
		{
			glEnable(cap);
		}
		else
		{
			glDisable(cap);
		}
		flags[index] = enable;
	}
}

//==============================================================================
Bool GlState::isEnabled(GLenum cap)
{
	U index = getIndexFromGlEnum(cap);
	Bool state = flags[index];
	ANKI_ASSERT(glIsEnabled(cap) == state);
	return state;
}

//==============================================================================
void GlState::sync()
{
	// Set flags
	for(U i = 0; i < glCaps.getSize(); i++)
	{
		flags[i] = glIsEnabled(glCaps[i]);
	}

	// viewport
	glGetIntegerv(GL_VIEWPORT, &viewport[0]);

	// clear color
	glGetFloatv(GL_COLOR_CLEAR_VALUE, &clearColor[0]);

	// clear depth value
	glGetFloatv(GL_DEPTH_CLEAR_VALUE, &clearDepthValue);

	// clear stencil value
	glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &clearStencilValue);

	// blend funcs
	// XXX fix that. Separate alpha with RGB
	glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&blendFuncs[0]);
	glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&blendFuncs[1]);

	// depth mask
	glGetIntegerv(GL_DEPTH_WRITEMASK, &depthMask);
}

//==============================================================================
void GlState::setViewport(U32 x, U32 y, U32 w, U32 h)
{
	if(x != (U32)viewport[0] 
		|| y != (U32)viewport[1]
		|| w != (U32)viewport[2] 
		|| h != (U32)viewport[3])
	{
		glViewport(x, y, w, h);
		viewport[0] = x;
		viewport[1] = y;
		viewport[2] = w;
		viewport[3] = h;
	}
}

//==============================================================================
void GlState::setClearColor(const Vec4& color)
{
#if ANKI_DEBUG
	Vec4 real;
	glGetFloatv(GL_COLOR_CLEAR_VALUE, &real[0]);
	ANKI_ASSERT(real == clearColor);
#endif

	if(clearColor != color)
	{
		glClearColor(color.x(), color.y(), color.z(), color.w());
		clearColor == color;
	}
}

//==============================================================================
void GlState::setClearDepthValue(const GLfloat depth)
{
#if ANKI_DEBUG
	GLfloat real;
	glGetFloatv(GL_DEPTH_CLEAR_VALUE, &real);
	ANKI_ASSERT(real == clearDepthValue);
#endif

	if(clearDepthValue != depth)
	{
		glClearDepthf(depth);
		clearDepthValue = depth;
	}
}

//==============================================================================
void GlState::setClearStencilValue(const GLint s)
{
#if ANKI_DEBUG
	GLint real;
	glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &real);
	ANKI_ASSERT(real == clearStencilValue);
#endif

	if(clearStencilValue != s)
	{
		glClearStencil(s);
		clearStencilValue = s;
	}
}

//==============================================================================
void GlState::setBlendFunctions(const GLenum sFactor, const GLenum dFactor)
{
#if ANKI_DEBUG
	GLint real;
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &real);
	ANKI_ASSERT(blendFuncs[0] == (GLenum)real);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &real);
	ANKI_ASSERT(blendFuncs[1] == (GLenum)real);
#endif

	if(sFactor != blendFuncs[0] || dFactor != blendFuncs[1])
	{
		glBlendFunc(sFactor, dFactor);
		blendFuncs[0] = sFactor;
		blendFuncs[1] = dFactor;
	}
}

//==============================================================================
void GlState::setDepthMaskEnabled(const Bool enable)
{
#if ANKI_DEBUG
	GLint real;
	glGetIntegerv(GL_DEPTH_WRITEMASK, &real);
	ANKI_ASSERT(real == depthMask);
#endif

	if(depthMask != enable)
	{
		glDepthMask(enable);
		depthMask = enable;
	}
}

} // end namespace anki
