#ifndef ANKI_GL_GL_STATE_H
#define ANKI_GL_GL_STATE_H

#include "anki/util/Singleton.h"
#include "anki/gl/Ogl.h"
#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include <unordered_map>
#include <mutex>
#include <algorithm>

namespace anki {

class Vao;
class Fbo;
class ShaderProgram;
class GlState;

/// @addtogroup gl
/// @{

/// Access the GL state machine.
/// This class saves us from calling the GL functions
class GlState
{
public:
	GlState()
	{}

	~GlState()
	{}

	void init(const U32 major_, const U32 minor_)
	{
		sync();
		major = major_;
		minor = minor_;
#if ANKI_DEBUG
		ANKI_ASSERT(initialized == false);
		initialized = true;
#endif
	}

	U32 getMajorVersion() const
	{
		return major;
	}
	U32 getMinorVersion() const
	{
		return minor;
	}

	/// @name Set the Fixed Function Pipeline, Call the OpenGL functions
	/// only when needed
	/// @{
	void enable(GLenum flag, bool enable = true);
	void disable(GLenum flag)
	{
		enable(flag, false);
	}
	bool isEnabled(GLenum flag);

	void setViewport(U32 x, U32 y, U32 w, U32 h);
	/// @}

	/// Set the current program
	void setProgram(ShaderProgram* prog);

	/// Set the current vao
	void setVao(Vao* vao);

	/// Set the current FBO
	/// @param fbo If nullptr then bind the default FBO
	void setFbo(Fbo* fbo);

	/// @name Draw functions
	/// @{
	void drawElements(U32 count);
	/// @}

private:
	/// Minor major GL version
	U32 major, minor;

	/// @name The GL state
	/// @{
	std::unordered_map<GLenum, bool> flags;
	static GLenum flagEnums[];

	GLint viewportX;
	GLint viewportY;
	GLsizei viewportW;
	GLsizei viewportH;
	/// @}

#if ANKI_DEBUG
	Bool initialized = false;
#endif

	/// Sync the local members with the opengl ones
	void sync();
};

typedef SingletonThreadSafe<GlState> GlStateSingleton;
/// @}

} // end namespace

#endif
