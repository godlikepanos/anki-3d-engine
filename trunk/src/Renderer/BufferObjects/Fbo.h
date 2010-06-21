#ifndef _FBO_H_
#define _FBO_H_

#include <GL/glew.h>
#include "Common.h"


/**
 * The class is actually a wrapper to avoid common mistakes
 */
class Fbo
{
	PROPERTY_R(uint, glId, getGlId) ///< OpenGL identification

	public:
		Fbo();

		/**
		 * Creates a new FBO
		 */
		void create();

		/**
		 * Binds FBO
		 */
		void bind() const;

		/**
		 * Unbinds the FBO. Actually unbinds all FBOs
		 */
		static void unbind();

		/**
		 * Checks the status of an initialized FBO
		 * @return True if FBO is ok and false if not
		 */
		bool isGood() const;

		/**
		 * Set the number of color attachements of the FBO
		 */
		void setNumOfColorAttachements(uint num) const;

		/**
		 * Returns the GL id of the current attached FBO
		 * @return Returns the GL id of the current attached FBO
		 */
		static uint getCurrentFbo();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline Fbo::Fbo():
	glId(0)
{}


inline void Fbo::create()
{
	DEBUG_ERR(glId != 0); // FBO already initialized
	glGenFramebuffers(1, &glId);
}


inline void Fbo::bind() const
{
	DEBUG_ERR(glId == 0);  // FBO not initialized
	glBindFramebuffer(GL_FRAMEBUFFER, glId);
}


inline void Fbo::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


inline bool Fbo::isGood() const
{
	DEBUG_ERR(glId == 0);  // FBO not initialized
	DEBUG_ERR(getCurrentFbo() != glId); // another FBO is binded

	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}


inline void Fbo::setNumOfColorAttachements(uint num) const
{
	DEBUG_ERR(glId == 0);  // FBO not initialized
	DEBUG_ERR(getCurrentFbo() != glId); // another FBO is binded

	if(num == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		static GLenum colorAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
																				 GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
																				 GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };

		DEBUG_ERR(num > sizeof(colorAttachments)/sizeof(GLenum));
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
