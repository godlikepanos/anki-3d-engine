#ifndef ANKI_GL_GL_STATE_H
#define ANKI_GL_GL_STATE_H

#include "anki/util/Singleton.h"
#include "anki/gl/Ogl.h"
#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/Math.h"

namespace anki {

class Vao;
class Fbo;
class ShaderProgram;

/// @addtogroup OpenGL
/// @{

/// Common stuff for all GL states
class GlStateCommon
{
public:
	/// Knowing the ventor allows some optimizations
	enum Gpu
	{
		GPU_UNKNOWN,
		GPU_ARM,
		GPU_NVIDIA
	};

	/// Return something like 430
	U32 getVersion() const
	{
		ANKI_ASSERT(version != -1);
		return version;
	}

	Gpu getGpu() const
	{
		return gpu;
	}

	Bool isTessellationSupported() const
	{
		return version >= 400;
	}

	Bool isComputeShaderSupported() const
	{
		return version >= 430;
	}

	void init(const U32 major, const U32 minor, 
		Bool registerDebugMessages = false);

private:
	/// Minor major GL version
	I32 version = -1;
	Gpu gpu = GPU_UNKNOWN;
};

typedef Singleton<GlStateCommon> GlStateCommonSingleton;

/// Access the GL state machine.
/// This class saves us from calling the GL functions
class GlState
{
public:
	GlState()
	{
		sync();
	}

	~GlState()
	{}

	/// @name Alter the GL state when needed
	/// @{
	void enable(GLenum cap, Bool enable = true);
	void disable(GLenum cap)
	{
		enable(cap, false);
	}
	bool isEnabled(GLenum cap);

	void setViewport(U32 x, U32 y, U32 w, U32 h);

	void setClearColor(const Vec4& color);

	void setClearDepthValue(const GLfloat depth);

	void setClearStencilValue(const GLint s);

	void setBlendFunctions(const GLenum sFactor, const GLenum dFactor);

	void setDepthMaskEnabled(const Bool enable);
	void setDepthFunc(const GLenum val);

	void setPolygonMode(const GLenum mode);
	/// @}

private:
	/// @name The GL state
	/// @{
	Array<Bool, 7> flags;
	Array<GLint, 4> viewport;
	GLfloat clearDepthValue;
	GLint clearStencilValue;
	Array<GLenum, 2> blendFuncs;

	Array<GLint, 4> colorMask;
	GLint depthMask;
	GLenum depthFunc;

#if ANKI_GL == ANKI_GL_DESKTOP
	GLenum polyMode;
#endif
	/// @}

	/// Sync the local members with the opengl ones
	void sync();

	static U getIndexFromGlEnum(const GLenum cap);
};

typedef SingletonThreadSafe<GlState> GlStateSingleton;
/// @}

} // end namespace anki

#endif
