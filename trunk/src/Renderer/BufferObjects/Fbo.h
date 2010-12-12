#ifndef FBO_H
#define FBO_H

#include <GL/glew.h>
#include "Exception.h"
#include "StdTypes.h"


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

		/// Creates a new FBO
		void create();

		/// Destroy it
		void destroy();

	private:
		uint glId; ///< OpenGL identification

		bool isCreated() const {return glId != 0;}
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline void Fbo::create()
{
	RASSERT_THROW_EXCEPTION(isCreated());
	glGenFramebuffers(1, &glId);
}


inline uint Fbo::getGlId() const
{
	RASSERT_THROW_EXCEPTION(isCreated());
	return glId;
}


inline Fbo::~Fbo()
{
	if(isCreated())
	{
		destroy();
	}
}


inline void Fbo::bind() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	glBindFramebuffer(GL_FRAMEBUFFER, glId);
}


inline void Fbo::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


inline void Fbo::checkIfGood() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	RASSERT_THROW_EXCEPTION(getCurrentFbo() != glId); // another FBO is binded

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw EXCEPTION("FBO is incomplete");
	}
}


inline void Fbo::setNumOfColorAttachements(uint num) const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	RASSERT_THROW_EXCEPTION(getCurrentFbo() != glId); // another FBO is binded

	if(num == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		static GLenum colorAttachments[] = {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
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


inline void Fbo::destroy()
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	glDeleteFramebuffers(1, &glId);
}


#endif
