#ifndef ANKI_GL_FBO_H
#define ANKI_GL_FBO_H

#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/util/StdTypes.h"
#include "anki/gl/Ogl.h"
#include <initializer_list>

namespace anki {

class Texture;

/// @addtogroup gl
/// @{

/// Frame buffer object. The class is actually a wrapper to avoid common 
/// mistakes. It only supports binding at both draw and read targets
class Fbo: public ContextNonSharable
{
public:
	/// FBO target
	enum FboTarget
	{
		FT_DRAW = 1 << 1,
		FT_READ = 1 << 2,
		FT_ALL = FT_DRAW | FT_READ
	};

	/// @name Constructors/Destructor
	/// @{
	Fbo()
	{}

	~Fbo();
	/// @}

	/// @name Accessors
	/// @{
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glId;
	}
	/// @}

	/// Binds FBO
	void bind(const FboTarget target = FT_ALL) const;

	/// Unbind all targets. Unbinds both draw and read FBOs so the active is
	/// the default FBO
	static void bindDefault(const FboTarget target = FT_ALL);

	/// Returns true if the FBO is ready for draw calls
	Bool isComplete() const;

	/// Set the color attachments of this FBO
	void setColorAttachments(
		const std::initializer_list<const Texture*>& textures);

	/// Set other attachment
	void setOtherAttachment(GLenum attachment, const Texture& tex, 
		const I32 layer = -1, const I32 face = -1);

	/// Creates a new FBO
	void create();

	/// Destroy it
	void destroy();

private:
	static thread_local const Fbo* currentRead;
	static thread_local const Fbo* currentDraw;
	GLuint glId = 0; ///< OpenGL identification

	Bool isCreated() const
	{
		return glId != 0;
	}

	static GLuint getCurrentFboGlId()
	{
		GLint i;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &i);
		return i;
	}

	static GLuint getCurrentDrawFboGlId()
	{
		GLint i;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &i);
		return i;
	}

	static GLuint getCurrentReadFboGlId()
	{
		GLint i;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &i);
		return i;
	}
};
/// @}

} // end namespace anki

#endif
