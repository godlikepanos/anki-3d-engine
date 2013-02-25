#ifndef ANKI_GL_FBO_H
#define ANKI_GL_FBO_H

#include "anki/gl/GlObject.h"
#include <initializer_list>

namespace anki {

class Texture;

/// @addtogroup OpenGL
/// @{

/// Frame buffer object. The class is actually a wrapper to avoid common 
/// mistakes. It only supports binding at both draw and read targets
class Fbo: public GlObjectContextNonSharable
{
public:
	typedef GlObjectContextNonSharable Base;

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

	/// Move
	Fbo(Fbo&& b)
	{
		*this = std::move(b);
	}

	~Fbo()
	{
		destroy();
	}
	/// @}

	/// @name Operators
	/// @{
	Fbo& operator=(Fbo&& b)
	{
		destroy();
		Base::operator=(std::forward<Base>(b));
		return *this;
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

	/// Blit framebuffer
	void blit(const Fbo& source, U32 srcX0, U32 srcY0, U32 srcX1, U32 srcY1, 
		U32 dstX0, U32 dstY0, U32 dstX1, U32 dstY1, GLbitfield mask,
		GLenum filter)
	{
		source.bind(FT_READ);
		bind(FT_DRAW);
		glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, 
			dstY1, mask, filter);
	}

	/// Creates a new FBO
	void create();

	/// Destroy it
	void destroy();

private:
	static thread_local const Fbo* currentRead;
	static thread_local const Fbo* currentDraw;

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
