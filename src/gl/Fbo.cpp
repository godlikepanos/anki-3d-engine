#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"
#include "anki/util/Array.h"

namespace anki {

//==============================================================================

thread_local const Fbo* Fbo::current = nullptr;

static const Array<GLenum, 8> colorAttachments = {{
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
	ANKI_GL_NON_SHARABLE_INIT();
	glGenFramebuffers(1, &glId);
	ANKI_ASSERT(glId != 0);
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
	ANKI_GL_NON_SHARABLE_CHECK();

	if(current != this)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, glId);
		current = this;
	}
}

//==============================================================================
void Fbo::unbind() const
{
	ANKI_ASSERT(isCreated());
	ANKI_GL_NON_SHARABLE_CHECK();

	if(current == this)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		current = nullptr;
	}
}

//==============================================================================
void Fbo::unbindAll()
{
	if(current != nullptr)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		current = nullptr;
	}
}

//==============================================================================
bool Fbo::isComplete() const
{
	bind();

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	return status == GL_FRAMEBUFFER_COMPLETE;
}

//==============================================================================
void Fbo::setColorAttachments(const std::initializer_list<const Texture*>& 
	textures)
{
	ANKI_ASSERT(textures.size() > 0);

	bind();

	// Set number of color attachments
	glDrawBuffers(textures.size(), &colorAttachments[0]);

	// Set attachments
	int i = 0;
	for(const Texture* tex : textures)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			GL_TEXTURE_2D, tex->getGlId(), 0);
		++i;
	}
}

//==============================================================================
void Fbo::setOtherAttachment(GLenum attachment, const Texture& tex, 
	const I32 layer, const I32 face)
{
	bind();

	const GLenum target = GL_FRAMEBUFFER;
	switch(tex.getTarget())
	{
	case GL_TEXTURE_2D:
		ANKI_ASSERT(layer < 0 && face < 0);
		glFramebufferTexture2D(target, attachment,
			GL_TEXTURE_2D, tex.getGlId(), 0);
		break;
	case GL_TEXTURE_CUBE_MAP:
		ANKI_ASSERT(layer < 0 && face >= 0);
		glFramebufferTexture2D(target, attachment,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex.getGlId(), 0);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		ANKI_ASSERT(layer >= 0 && face < 0);
		ANKI_ASSERT((GLuint)layer < tex.getDepth());
		glFramebufferTextureLayer(target, attachment,
			tex.getGlId(), 0, layer);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

} // end namespace anki
