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
	*static_cast<FramebufferInitializer*>(this) = init;

	if(m_colorAttachmentsCount == 0 
		&& !m_depthStencilAttachment.m_texture.isCreated())
	{
		m_bindDefault = true;
		return ErrorCode::NONE;
	}

	m_bindDefault = false;

	glGenFramebuffers(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);
	const GLenum target = GL_FRAMEBUFFER;
	glBindFramebuffer(target, m_glName);

	// Attach color
	for(U i = 0; i < m_colorAttachmentsCount; i++)
	{
		const Attachment& att = m_colorAttachments[i];
		const GLenum binding = GL_COLOR_ATTACHMENT0 + i;

		attachTextureInternal(binding, att.m_texture.get(), att.m_layer);

		m_drawBuffers[i] = binding;

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}
	}

	// Attach depth/stencil
	if(m_depthStencilAttachment.m_texture.isCreated())
	{
		const Attachment& att = m_depthStencilAttachment;
		const GLenum binding = GL_DEPTH_ATTACHMENT;
			
		attachTextureInternal(binding, att.m_texture.get(), att.m_layer);

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}
	}

	// Check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		ANKI_LOGE("FBO is incomplete");
		destroy();
		return ErrorCode::FUNCTION_FAILED;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return ErrorCode::NONE;
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
void FramebufferImpl::bind()
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
		if(m_colorAttachmentsCount)
		{
			glDrawBuffers(m_colorAttachmentsCount, &m_drawBuffers[0]);
		}

		// Invalidate
		if(m_invalidateBuffersCount)
		{
			glInvalidateFramebuffer(GL_FRAMEBUFFER, m_invalidateBuffersCount, 
				&m_invalidateBuffers[0]);
		}

		ANKI_ASSERT(0 && "TODO clear buffers");
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


