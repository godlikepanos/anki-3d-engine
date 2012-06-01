#ifndef ANKI_GL_GL_STATE_H
#define ANKI_GL_GL_STATE_H

#include "anki/util/Singleton.h"
#include "anki/gl/Gl.h"
#include <unordered_map>

namespace anki {

/// Access the GL state machine.
/// This class saves us from calling the GL functions
class GlState
{
public:
	GlState()
	{
		sync();
	}

	/// Sync the local members with the opengl ones
	void sync();

	/// @name Set the Fixed Function Pipeline, Call the OpenGL functions
	/// only when needed
	/// @{
	void enable(GLenum flag, bool enable = true);
	void disable(GLenum flag)
	{
		enable(flag, false);
	}
	bool isEnabled(GLenum flag);

	void setViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	/// @}

private:
	/// @name The GL state
	/// @{
	std::unordered_map<GLenum, bool> flags;
	static GLenum flagEnums[];

	GLint viewportX;
	GLint viewportY;
	GLsizei viewportW;
	GLsizei viewportH;
	/// @}
};

typedef SingletonThreadSafe<GlState> GlStateSingleton;

} // end namespace

#endif
