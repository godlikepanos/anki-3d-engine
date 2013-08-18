#include "anki/gl/GlState.h"
#include "anki/gl/ShaderProgram.h"
#include "anki/gl/Fbo.h"
#include "anki/gl/Vao.h"
#include "anki/util/Assert.h"
#include "anki/core/Logger.h"
#include <limits>
#include <algorithm>

namespace anki {

//==============================================================================
// GlStateCommon                                                               = 
//==============================================================================

struct GlDbg
{
	GLenum token;
	const char* str;
};

static const GlDbg gldbgsource[] = {
	{GL_DEBUG_SOURCE_API, "GL_DEBUG_SOURCE_API"},
	{GL_DEBUG_SOURCE_WINDOW_SYSTEM, "GL_DEBUG_SOURCE_WINDOW_SYSTEM"},
	{GL_DEBUG_SOURCE_SHADER_COMPILER, "GL_DEBUG_SOURCE_SHADER_COMPILER"}, 
	{GL_DEBUG_SOURCE_THIRD_PARTY, "GL_DEBUG_SOURCE_THIRD_PARTY"},
	{GL_DEBUG_SOURCE_APPLICATION, "GL_DEBUG_SOURCE_APPLICATION"},
	{GL_DEBUG_SOURCE_OTHER, "GL_DEBUG_SOURCE_OTHER"}};

static const GlDbg gldbgtype[] = {
	{GL_DEBUG_TYPE_ERROR, "GL_DEBUG_TYPE_ERROR"},
	{GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"},
	{GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"},
	{GL_DEBUG_TYPE_PORTABILITY, "GL_DEBUG_TYPE_PORTABILITY"},
	{GL_DEBUG_TYPE_PERFORMANCE, "GL_DEBUG_TYPE_PERFORMANCE"},
	{GL_DEBUG_TYPE_OTHER, "GL_DEBUG_TYPE_OTHER"}};

static const GlDbg gldbgseverity[] = {
	{GL_DEBUG_SEVERITY_LOW, "GL_DEBUG_SEVERITY_LOW"},
	{GL_DEBUG_SEVERITY_MEDIUM, "GL_DEBUG_SEVERITY_MEDIUM"},
	{GL_DEBUG_SEVERITY_HIGH, "GL_DEBUG_SEVERITY_HIGH"}};

//==============================================================================
static void oglMessagesCallback(GLenum source,
	GLenum type, GLuint id, GLenum severity, GLsizei length,
	const char* message, GLvoid* userParam)
{
	const GlDbg* sourced = &gldbgsource[0];
	while(source != sourced->token)
	{
		++sourced;
	}

	const GlDbg* typed = &gldbgtype[0];
	while(type != typed->token)
	{
		++typed;
	}

	switch(severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		ANKI_LOGI("GL: " << sourced->str 
			<< ", " << typed->str << ": " << message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		ANKI_LOGW("GL: " << sourced->str 
			<< ", " << typed->str << ": " << message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		ANKI_LOGE("GL: " << sourced->str 
			<< ", " << typed->str << ": " << message);
		break;
	}
}

//==============================================================================
void GlStateCommon::init(const U32 major_, const U32 minor_, 
	Bool registerDebugMessages)
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

	// Enable debug messages
	if(registerDebugMessages)
	{
		glDebugMessageCallback(oglMessagesCallback, nullptr);

		for(U s = 0; s < sizeof(gldbgsource) / sizeof(GlDbg); s++)
		{
			for(U t = 0; t < sizeof(gldbgtype) / sizeof(GlDbg); t++)
			{
				for(U sv = 0; sv < sizeof(gldbgseverity) / sizeof(GlDbg); sv++)
				{
					glDebugMessageControl(gldbgsource[s].token, 
						gldbgtype[t].token, gldbgseverity[sv].token, 0, 
						nullptr, GL_TRUE);
				}
			}
		}
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
void GlState::enable(GLenum cap, Bool enable)
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
	// depth func
	glGetIntegerv(GL_DEPTH_FUNC, (GLint*)&depthFunc);
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
	glClearColor(color.x(), color.y(), color.z(), color.w());
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

//==============================================================================
void GlState::setDepthFunc(const GLenum val)
{
#if ANKI_DEBUG
	GLint real;
	glGetIntegerv(GL_DEPTH_FUNC, &real);
	ANKI_ASSERT((GLenum)real == depthFunc);
#endif

	if(val != depthFunc)
	{
		glDepthFunc(val);
		depthFunc = val;
	}
}

} // end namespace anki
