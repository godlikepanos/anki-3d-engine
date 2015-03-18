// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/FramebufferImpl.h"
#include "anki/gr/FramebufferCommon.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
Error FramebufferImpl::create(Initializer& init)
{
	if(init.m_colorAttachmentsCount == 0 
		&& !init.m_depthStencilAttachment.m_texture.isCreated())
	{
		m_bindDefault = true;
		return ErrorCode::NONE;
	}

	m_bindDefault = false;

	Array<U, MAX_COLOR_ATTACHMENTS + 1> layers;
	GLenum depthStencilBindingPoint = GL_NONE;

	// Do color attachments
	for(U i = 0; i < init.m_colorAttachmentsCount; ++i)
	{
		auto& attachment = init.m_colorAttachments[i];
		ANKI_ASSERT(attachment.m_texture.isCreated());
		m_attachments[i] = attachment.m_texture;
		layers[i] = attachment.m_layer;
	}

	// Do depth stencil
	if(init.m_depthStencilAttachment.m_texture.isCreated())
	{
		m_attachments[MAX_COLOR_ATTACHMENTS] = 
			init.m_depthStencilAttachment.m_texture;
		layers[MAX_COLOR_ATTACHMENTS] = 
			init.m_depthStencilAttachment.m_layer;

		depthStencilBindingPoint = GL_DEPTH_STENCIL_ATTACHMENT;
	}

	// Now create the FBO
	Error err = createFbo(layers, depthStencilBindingPoint);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return err;
}

//==============================================================================
void FramebufferImpl::destroy()
{
	if(m_glName != 0)
	{
		glDeleteFramebuffers(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
Error FramebufferImpl::createFbo(
	const Array<U, MAX_COLOR_ATTACHMENTS + 1>& layers,
	GLenum depthStencilBindingPoint)
{
	Error err = ErrorCode::NONE;

	ANKI_ASSERT(!isCreated());
	glGenFramebuffers(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);
	const GLenum target = GL_FRAMEBUFFER;
	glBindFramebuffer(target, m_glName);

	// Attach color
	for(U i = 0; i < MAX_COLOR_ATTACHMENTS; i++)
	{
		if(!m_attachments[i].isCreated())
		{
			continue;
		}
		
		const TextureImpl& tex = m_attachments[i].get();
		attachTextureInternal(GL_COLOR_ATTACHMENT0 + i, tex, layers[i]);
	}

	// Attach depth/stencil
	if(m_attachments[MAX_COLOR_ATTACHMENTS].isCreated())
	{
		ANKI_ASSERT(depthStencilBindingPoint != GL_NONE);

		const TextureImpl& tex = m_attachments[MAX_COLOR_ATTACHMENTS].get();
		attachTextureInternal(depthStencilBindingPoint, tex, 
			layers[MAX_COLOR_ATTACHMENTS]);
	}

	// Check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		ANKI_LOGE("FBO is incomplete");
		destroy();
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

//==============================================================================
void FramebufferImpl::attachTextureInternal(
	GLenum attachment, const TextureImpl& tex, const U32 layer)
{
	const GLenum target = GL_FRAMEBUFFER;
	switch(tex.getTarget())
	{
	case GL_TEXTURE_2D:
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_TEXTURE_2D_MULTISAMPLE:
#endif
		ANKI_ASSERT(layer == 0);
		glFramebufferTexture2D(target, attachment,
			tex.getTarget(), tex.getGlName(), 0);
		break;
	case GL_TEXTURE_CUBE_MAP:
		ANKI_ASSERT(layer < 6);
		glFramebufferTexture2D(target, attachment,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, tex.getGlName(), 0);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		ANKI_ASSERT((GLuint)layer < tex.getDepth());
		glFramebufferTextureLayer(target, attachment,
			tex.getGlName(), 0, layer);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

//==============================================================================
void FramebufferImpl::bind(Bool invalidate)
{
	if(m_bindDefault)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		ANKI_ASSERT(m_glName != 0);
		glBindFramebuffer(GL_FRAMEBUFFER, m_glName);

		// Set the draw buffers
		U count = 0;
		Array<GLenum, MAX_COLOR_ATTACHMENTS> colorAttachEnums;

		// Draw buffers
		for(U i = 0; i < MAX_COLOR_ATTACHMENTS; i++)
		{
			if(m_attachments[i].isCreated())
			{
				colorAttachEnums[count++] = GL_COLOR_ATTACHMENT0 + i;
			}
		}
		ANKI_ASSERT(count > 0
			|| m_attachments[MAX_COLOR_ATTACHMENTS].isCreated());
		glDrawBuffers(count, &colorAttachEnums[0]);

		// Invalidate
		if(invalidate)
		{
			static const Array<GLenum, MAX_COLOR_ATTACHMENTS + 1>
				ATTACHMENT_TOKENS = {{
					GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0, 
					GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, 
					GL_COLOR_ATTACHMENT3}};

			glInvalidateFramebuffer(
				GL_FRAMEBUFFER, ATTACHMENT_TOKENS.size(), 
				&ATTACHMENT_TOKENS[0]);
		}
	}
}

//==============================================================================
void FramebufferImpl::blit(const FramebufferImpl& b, 
	const Array<U32, 4>& sourceRect,
	const Array<U32, 4>& destRect, 
	GLbitfield attachmentMask, Bool linear)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_glName);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, b.m_glName);
	glBlitFramebuffer(
		sourceRect[0], sourceRect[1], sourceRect[2], sourceRect[3],
		destRect[0], destRect[1], destRect[2], destRect[3], 
		attachmentMask,
		linear ? GL_LINEAR : GL_NEAREST);
}

} // end namespace anki


