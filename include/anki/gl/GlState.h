#ifndef ANKI_GL_GL_STATE_H
#define ANKI_GL_GL_STATE_H

#include "anki/util/Singleton.h"
#include "anki/gl/Ogl.h"
#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/math/Math.h"

namespace anki {

class Vao;
class Fbo;
class ShaderProgram;

/// @addtogroup gl
/// @{

/// Common stuff for all states
class GlStateCommon
{
public:
	U32 getMajorVersion() const
	{
		ANKI_ASSERT(major != -1);
		return (U32)major;
	}
	U32 getMinorVersion() const
	{
		ANKI_ASSERT(minor != -1);
		return (U32)minor;
	}

	void init(const U32 major_, const U32 minor_)
	{
		major = (I32)major_;
		minor = (I32)minor_;
	}

private:
	/// Minor major GL version
	I32 major = -1, minor = -1;
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
	/// @}

private:
	/// @name The GL state
	/// @{
	Array<Bool, 7> flags;
	Array<GLint, 4> viewport;
	Vec4 clearColor;
	GLfloat clearDepthValue;
	GLint clearStencilValue;
	Array<GLenum, 2> blendFuncs;

	// XXX
	GLenum depthFunc; 
	Array<GLint, 4> colorMask;
	GLint depthMask;
	/// @}

	/// Sync the local members with the opengl ones
	void sync();

	static U getIndexFromGlEnum(const GLenum cap);
};

typedef SingletonThreadSafe<GlState> GlStateSingleton;
/// @}

} // end namespace anki

#endif
