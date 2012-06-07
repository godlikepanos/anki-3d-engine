#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"
#include <array>

namespace anki {

//==============================================================================

std::atomic<uint32_t> Fbo::counter(0);

static const std::array<GLenum, 8> colorAttachments = {{
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
void Fbo::create()
{
	ANKI_ASSERT(!isCreated());
	glGenFramebuffers(1, &glId);
	ANKI_ASSERT(glId != 0);
	uuid = counter.fetch_add(1, std::memory_order_relaxed);
}

//==============================================================================
void Fbo::destroy()
{
	ANKI_ASSERT(isCreated());
	unbind();
	glDeleteFramebuffers(1, &glId);
	glId = 0;
}

//==============================================================================
void Fbo::bind() const
{
	ANKI_ASSERT(isCreated());
	glBindFramebuffer(GL_FRAMEBUFFER, glId);
}

//==============================================================================
void Fbo::unbind() const
{
	ANKI_ASSERT(isCreated());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//==============================================================================
void Fbo::unbindAll()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//==============================================================================
bool Fbo::isComplete() const
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(glId == getCurrentDrawFboGlId() && "Not binded");

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	return status == GL_FRAMEBUFFER_COMPLETE;
}

//==============================================================================
void Fbo::setColorAttachments(const std::initializer_list<const Texture*>& 
	textures)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(glId == getCurrentDrawFboGlId() && "Not binded");
	ANKI_ASSERT(textures.size() > 0);

	// Set number of color attachments
	//
	glDrawBuffers(textures.size(), &colorAttachments[0]);

	// Set attachments
	//
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
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(glId == getCurrentDrawFboGlId() && "Not binded");

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
		GL_TEXTURE_2D, tex.getGlId(), 0);
}

} // end namespace
