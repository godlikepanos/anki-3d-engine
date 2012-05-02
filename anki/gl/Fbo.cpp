#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"
#include <boost/lexical_cast.hpp>
#include <array>


namespace anki {


//==============================================================================

static std::array<GLenum, 8> colorAttachments = {{
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7}};


//==============================================================================
Fbo::~Fbo()
{
	if(isCreated())
	{
		destroy();
	}
}


//==============================================================================
void Fbo::checkIfGood() const
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(getCurrentFbo() == glId); // another FBO is binded

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw ANKI_EXCEPTION("FBO is incomplete: " 
			+ boost::lexical_cast<std::string>(status));
	}
}


//==============================================================================
void Fbo::setNumOfColorAttachements(uint num) const
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(getCurrentFbo() == glId); // another FBO is binded

	if(num == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		ANKI_ASSERT(num <= colorAttachments.size());
		glDrawBuffers(num, &colorAttachments[0]);
	}
}


//==============================================================================
uint Fbo::getCurrentFbo()
{
	int fboGlId;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fboGlId);
	return (uint)fboGlId;
}


//==============================================================================
void Fbo::setColorAttachments(const std::initializer_list<const Texture*>& 
	textures)
{
	setNumOfColorAttachements(textures.size());
	int i = 0;
	for(const Texture* tex : textures)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			GL_TEXTURE_2D, tex->getGlId(), 0);
		++i;
	}
}


//==============================================================================
void Fbo::setOtherAttachment(GLenum attachment, const Texture& tex)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
		GL_TEXTURE_2D, tex.getGlId(), 0);
}


} // end namespace
