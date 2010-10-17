#ifndef FBO_H
#define FBO_H

#include <GL/glew.h>
#include "Exception.h"
#include "Properties.h"


/// The class is actually a wrapper to avoid common mistakes
class Fbo
{
	PROPERTY_R(uint, glId, getGlId) ///< OpenGL identification

	public:
		Fbo(): glId(0) {}

		/// Creates a new FBO
		void create();

		/// Binds FBO
		void bind() const;

		/// Unbinds the FBO. Actually unbinds all FBOs
		static void unbind();

		/// Checks the status of an initialized FBO and if fails throw exception
		/// @exception Exception
		void checkIfGood() const;

		/// Set the number of color attachements of the FBO
		void setNumOfColorAttachements(uint num) const;

		/// Returns the GL id of the current attached FBO
		/// @return Returns the GL id of the current attached FBO
		static uint getCurrentFbo();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline void Fbo::create()
{
	RASSERT_THROW_EXCEPTION(glId != 0); // FBO already initialized
	glGenFramebuffers(1, &glId);
}


inline void Fbo::bind() const
{
	RASSERT_THROW_EXCEPTION(glId == 0);  // FBO not initialized
	glBindFramebuffer(GL_FRAMEBUFFER, glId);
}


inline void Fbo::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


inline void Fbo::checkIfGood() const
{
	RASSERT_THROW_EXCEPTION(glId == 0);  // FBO not initialized
	RASSERT_THROW_EXCEPTION(getCurrentFbo() != glId); // another FBO is binded

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw EXCEPTION("FBO is incomplete");
	}
}


inline void Fbo::setNumOfColorAttachements(uint num) const
{
	RASSERT_THROW_EXCEPTION(glId == 0);  // FBO not initialized
	RASSERT_THROW_EXCEPTION(getCurrentFbo() != glId); // another FBO is binded

	if(num == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		static GLenum colorAttachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
																				GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
																				GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};

		RASSERT_THROW_EXCEPTION(num > sizeof(colorAttachments)/sizeof(GLenum));
		glDrawBuffers(num, colorAttachments);
	}
}


inline uint Fbo::getCurrentFbo()
{
	int fboGlId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fboGlId);
	return (uint)fboGlId;
}


#endif
