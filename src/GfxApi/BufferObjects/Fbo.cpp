#include "Fbo.h"
#include <boost/lexical_cast.hpp>


//==============================================================================
// checkIfGood                                                                 =
//==============================================================================
void Fbo::checkIfGood() const
{
	ASSERT(isCreated());
	ASSERT(getCurrentFbo() == glId); // another FBO is binded

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw EXCEPTION("FBO is incomplete: " + boost::lexical_cast<std::string>(status));
	}
}


//==============================================================================
// setNumOfColorAttachements                                                   =
//==============================================================================
void Fbo::setNumOfColorAttachements(uint num) const
{
	ASSERT(isCreated());
	ASSERT(getCurrentFbo() == glId); // another FBO is binded

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

		ASSERT(num <= sizeof(colorAttachments) / sizeof(GLenum));
		glDrawBuffers(num, colorAttachments);
	}
}


//==============================================================================
// getCurrentFbo                                                               =
//==============================================================================
uint Fbo::getCurrentFbo()
{
	int fboGlId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fboGlId);
	return (uint)fboGlId;
}
