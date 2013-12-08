#include "anki/gl/Fbo.h"
#include "anki/gl/Texture.h"
#include "anki/gl/GlState.h"
#include "anki/util/Array.h"

namespace anki {

//==============================================================================

thread_local const Fbo* Fbo::currentRead = nullptr;
thread_local const Fbo* Fbo::currentDraw = nullptr;

static const Array<GLenum, Fbo::MAX_COLOR_ATTACHMENTS + 1> 
	ATTACHMENT_TOKENS = {{
	GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, 
	GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3}};

//==============================================================================
void Fbo::create(const std::initializer_list<Attachment>& attachments)
{
	ANKI_ASSERT(!isCreated());
	crateNonSharable();

	// Create name
	glGenFramebuffers(1, &glId);
	ANKI_ASSERT(glId != 0);

	// Bind
	bind(false, ALL_TARGETS);

	// Attach textures
	colorAttachmentsCount = 0;
	for(const Attachment& attachment : attachments)
	{
		// Set some values
		const GLenum max = GL_COLOR_ATTACHMENT0 + MAX_COLOR_ATTACHMENTS;
		if(attachment.attachmentPoint >= GL_COLOR_ATTACHMENT0 
			&& attachment.attachmentPoint < max)
		{
			ANKI_ASSERT(colorAttachmentsCount == 
				attachment.attachmentPoint - GL_COLOR_ATTACHMENT0);

			++colorAttachmentsCount;
		}
		else
		{
			ANKI_ASSERT(
				attachment.attachmentPoint == GL_DEPTH_STENCIL_ATTACHMENT
				|| attachment.attachmentPoint == GL_DEPTH_ATTACHMENT
				|| attachment.attachmentPoint == GL_STENCIL_ATTACHMENT);
		}

		attachTextureInternal(attachment.attachmentPoint, *attachment.texture,
			attachment.layer);
	}

	// Check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw ANKI_EXCEPTION("FBO is incomplete");
	}
}

//==============================================================================
void Fbo::destroy()
{
	checkNonSharable();
	if(isCreated())
	{
		bindDefault(false, ALL_TARGETS);
		glDeleteFramebuffers(1, &glId);
		glId = 0;
		colorAttachmentsCount = 0;
	}
}

//==============================================================================
void Fbo::bindInternal(const Fbo* fbo, Bool invalidate, const Target target)
{
	GLenum glTarget = GL_FRAMEBUFFER;
	GLuint name = fbo != nullptr ? fbo->glId : 0;

	// Bind
	if(target == ALL_TARGETS)
	{
		if(currentDraw != fbo || currentRead != fbo)
		{
			glBindFramebuffer(glTarget, name);
			currentDraw = currentRead = fbo;
		}
	}
	else if(target == DRAW_TARGET)
	{
		glTarget = GL_DRAW_FRAMEBUFFER;
		if(currentDraw != fbo)
		{
			glBindFramebuffer(glTarget, name);
			currentDraw = fbo;
		}
	}
	else
	{
		ANKI_ASSERT(target == READ_TARGET);

		glTarget = GL_READ_FRAMEBUFFER;
		if(currentRead != fbo)
		{
			glBindFramebuffer(glTarget, name);
			currentRead = fbo;
		}
	}

	// Set the drawbuffers
	if(name != 0)
	{
		glDrawBuffers(fbo->colorAttachmentsCount, 
			&ATTACHMENT_TOKENS[1]);

		// Invalidate
		if(invalidate)
		{
			glInvalidateFramebuffer(
				glTarget, ATTACHMENT_TOKENS.size(), &ATTACHMENT_TOKENS[0]);

			if(name == 0
				&& GlStateCommonSingleton::get().getGpu() == 
					GlStateCommon::GPU_ARM)
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
					| GL_STENCIL_BUFFER_BIT);
			}

#if ANKI_DEBUG == 1
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
				| GL_STENCIL_BUFFER_BIT);
#endif
		}
	}
}

//==============================================================================
void Fbo::attachTextureInternal(GLenum attachment, const Texture& tex, 
	const I32 layer)
{
	const GLenum target = GL_FRAMEBUFFER;
	switch(tex.getTarget())
	{
	case GL_TEXTURE_2D:
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_TEXTURE_2D_MULTISAMPLE:
#endif
		ANKI_ASSERT(layer < 0);
		glFramebufferTexture2D(target, attachment,
			tex.getTarget(), tex.getGlId(), 0);
		break;
	case GL_TEXTURE_CUBE_MAP:
		ANKI_ASSERT(layer >= 0 && layer < 6);
		glFramebufferTexture2D(target, attachment,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, tex.getGlId(), 0);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		ANKI_ASSERT(layer >= 0);
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
