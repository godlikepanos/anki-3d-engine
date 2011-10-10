#ifndef FBO_H
#define FBO_H

#include <GL/glew.h>
#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Exception.h"


/// The class is actually a wrapper to avoid common mistakes
class Fbo
{
	public:
		/// Constructor
		Fbo(): glId(0) {}

		/// Destructor
		~Fbo();

		uint getGlId() const;

		/// Binds FBO
		void bind(GLenum target = GL_FRAMEBUFFER);

		/// Unbinds the FBO
		void unbind();

		/// Checks the status of an initialized FBO and if fails throw exception
		/// @exception Exception
		void checkIfGood() const;

		/// Set the number of color attachments of the FBO
		void setNumOfColorAttachements(uint num) const;

		/// Returns the GL id of the current attached FBO
		/// @return Returns the GL id of the current attached FBO
		static uint getCurrentFbo();

		/// Creates a new FBO
		void create();

		/// Destroy it
		void destroy();

	private:
		uint glId; ///< OpenGL identification
		GLenum target; ///< How the buffer is bind

		bool isCreated() const {return glId != 0;}
};


//==============================================================================
// Inlines                                                                     =
//==============================================================================

inline void Fbo::create()
{
	ASSERT(!isCreated());
	glGenFramebuffers(1, &glId);
}


inline uint Fbo::getGlId() const
{
	ASSERT(!isCreated());
	return glId;
}


inline void Fbo::bind(GLenum target_)
{
	ASSERT(isCreated());
	target = target_;
	ASSERT(target == GL_DRAW_FRAMEBUFFER ||
		target == GL_READ_FRAMEBUFFER ||
		target == GL_FRAMEBUFFER);
	glBindFramebuffer(target, glId);
}


inline void Fbo::unbind()
{
	glBindFramebuffer(target, 0);
}


inline void Fbo::destroy()
{
	ASSERT(isCreated());
	glDeleteFramebuffers(1, &glId);
}


#endif
