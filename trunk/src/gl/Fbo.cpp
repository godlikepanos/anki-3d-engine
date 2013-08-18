#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"
#include "anki/util/Array.h"

namespace anki {

//==============================================================================

thread_local const Fbo* Fbo::currentRead = nullptr;
thread_local const Fbo* Fbo::currentDraw = nullptr;

static const Array<GLenum, 8> colorAttachments = {{
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7}};

//==============================================================================
void Fbo::create()
{
	ANKI_ASSERT(!isCreated());
	crateNonSharable();
	glGenFramebuffers(1, &glId);
	ANKI_ASSERT(glId != 0);
}

//==============================================================================
void Fbo::destroy()
{
	checkNonSharable();
	if(isCreated())
	{
		bindDefault();
		glDeleteFramebuffers(1, &glId);
		glId = 0;
	}
}

//==============================================================================
void Fbo::bind(const FboTarget target, Bool noReadbacks) const
{
	ANKI_ASSERT(isCreated());
	checkNonSharable();

	if(target == FT_ALL)
	{
		if(currentDraw != this || currentRead != this)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, glId);
			currentDraw = currentRead = this;
		}

#if ANKI_GL == ANKI_GL_ES
		if(noReadbacks)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT 
				| GL_STENCIL_BUFFER_BIT);
		}
#endif
	}
	else if(target == FT_DRAW)
	{
		if(currentDraw != this)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glId);
			currentDraw = this;
		}

#if ANKI_GL == ANKI_GL_ES
		if(noReadbacks)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT 
				| GL_STENCIL_BUFFER_BIT);
		}
#endif
	}
	else
	{
		ANKI_ASSERT(target == FT_READ);
		if(currentRead != this)
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, glId);
			currentRead = this;
		}

		ANKI_ASSERT(noReadbacks == false && "Dosn't make sense");
	}
}

//==============================================================================
void Fbo::bindDefault(const FboTarget target)
{
	if(target == FT_ALL)
	{
		if(currentDraw != nullptr || currentRead != nullptr)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			currentDraw = currentRead = nullptr;
		}
	}
	else if(target == FT_DRAW)
	{
		if(currentDraw != nullptr)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			currentDraw = nullptr;
		}
	}
	else
	{
		ANKI_ASSERT(target == FT_READ);
		if(currentRead != nullptr)
		{
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			currentRead = nullptr;
		}
	}
}

//==============================================================================
Bool Fbo::isComplete() const
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
	U i = 0;
	for(const Texture* tex : textures)
	{
#if ANKI_GL == ANKI_GL_DESKTOP
		ANKI_ASSERT(tex->getTarget() == GL_TEXTURE_2D 
			|| tex->getTarget() == GL_TEXTURE_2D_MULTISAMPLE);
#else
		ANKI_ASSERT(tex->getTarget() == GL_TEXTURE_2D);
#endif

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
			tex->getTarget(), tex->getGlId(), 0);

		attachments[attachmentsCount++] = GL_COLOR_ATTACHMENT0 + i;

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
	case GL_TEXTURE_2D_MULTISAMPLE:
		ANKI_ASSERT(layer < 0 && face < 0);
		glFramebufferTexture2D(target, attachment,
			tex.getTarget(), tex.getGlId(), 0);
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

	attachments[attachmentsCount++] = attachment;
}

} // end namespace anki
