#ifndef ANKI_GL_FBO_H
#define ANKI_GL_FBO_H

#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/gl/Gl.h"
#include <array>
#include <initializer_list>

namespace anki {

class Texture;

/// @addtogroup gl
/// @{

/// Frame buffer object. The class is actually a wrapper to avoid common 
/// mistakes. It only supports binding at both draw and read targets
class Fbo
{
public:
	/// @name Constructors/Destructor
	/// @{
	Fbo()
		: glId(0)
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
	void bind() const;

	/// Unbinds the FBO
	void unbind() const;

	/// Unbind all
	///
	/// Unbinds both draw and read FBOs so the active is the default FBO
	static void unbindAll();

	/// Checks the status of an initialized FBO and if fails throws exception
	/// @exception Exception
	void checkIfGood() const;

	/// Set the color attachments of this FBO
	void setColorAttachments(const std::initializer_list<const Texture*>& 
		textures);

	/// Set other attachment
	void setOtherAttachment(GLenum attachment, const Texture& tex);

	/// Creates a new FBO
	void create()
	{
		ANKI_ASSERT(!isCreated());
		glGenFramebuffers(1, &glId);
		ANKI_ASSERT(glId != 0);
	}

	/// Destroy it
	void destroy()
	{
		ANKI_ASSERT(isCreated());
		unbind();
		glDeleteFramebuffers(1, &glId);
		glId = 0;
	}

private:
	/// Current binded FBOs for draw/read
	static thread_local const Fbo* currentFbo;

	GLuint glId; ///< OpenGL identification

	bool isCreated() const
	{
		return glId != 0;
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

} // end namespace

#endif
