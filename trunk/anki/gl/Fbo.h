#ifndef ANKI_GL_FBO_H
#define ANKI_GL_FBO_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Exception.h"
#include <GL/glew.h>
#include <array>
#include <initializer_list>


namespace anki {


class Texture;


/// The class is actually a wrapper to avoid common mistakes
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

	uint getGlId() const
	{
		ANKI_ASSERT(!isCreated());
		return glId;
	}

	/// Binds FBO
	void bind(GLenum target_ = GL_FRAMEBUFFER)
	{
		ANKI_ASSERT(isCreated());
		target = target_;
		ANKI_ASSERT(target == GL_DRAW_FRAMEBUFFER
			|| target == GL_READ_FRAMEBUFFER
			|| target == GL_FRAMEBUFFER);
		glBindFramebuffer(target, glId);
	}

	/// Unbinds the FBO
	void unbind()
	{
		glBindFramebuffer(target, 0);
	}

	/// Checks the status of an initialized FBO and if fails throw exception
	/// @exception Exception
	void checkIfGood() const;

	/// Set the color attachments of this FBO
	void setColorAttachments(const std::initializer_list<const Texture*>& 
		textures);

	/// Set other attachment
	void setOtherAttachment(GLenum attachment, const Texture& tex);

	/// Returns the GL id of the current attached FBO
	/// @return Returns the GL id of the current attached FBO
	static uint getCurrentFbo();

	/// Creates a new FBO
	void create()
	{
		ANKI_ASSERT(!isCreated());
		glGenFramebuffers(1, &glId);
	}

	/// Destroy it
	void destroy()
	{
		ANKI_ASSERT(isCreated());
		glDeleteFramebuffers(1, &glId);
	}

private:
	uint glId; ///< OpenGL identification
	GLenum target; ///< How the buffer is bind

	bool isCreated() const
	{
		return glId != 0;
	}

	/// Set the number of color attachments of the FBO
	void setNumOfColorAttachements(uint num) const;
};


} // end namespace


#endif
