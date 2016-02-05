// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/util/Logger.h>

namespace anki
{

//==============================================================================
Error FramebufferImpl::create(const FramebufferInitInfo& init)
{
	ANKI_ASSERT(!isCreated());
	m_in = init;

	if(m_in.m_colorAttachmentsCount == 0
		&& !m_in.m_depthStencilAttachment.m_texture.isCreated())
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
	for(U i = 0; i < m_in.m_colorAttachmentsCount; i++)
	{
		const Attachment& att = m_in.m_colorAttachments[i];
		const GLenum binding = GL_COLOR_ATTACHMENT0 + i;

		attachTextureInternal(binding, att.m_texture->getImplementation(), att);

		m_drawBuffers[i] = binding;

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}
	}

	// Attach depth/stencil
	if(m_in.m_depthStencilAttachment.m_texture.isCreated())
	{
		const Attachment& att = m_in.m_depthStencilAttachment;
		const GLenum binding = GL_DEPTH_ATTACHMENT;

		attachTextureInternal(binding, att.m_texture->getImplementation(), att);

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}
	}

	// Check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		ANKI_LOGE("FBO is incomplete: 0x%x", status);
		return ErrorCode::FUNCTION_FAILED;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return ErrorCode::NONE;
}

//==============================================================================
void FramebufferImpl::attachTextureInternal(
	GLenum attachment, const TextureImpl& tex, const Attachment& info)
{
	ANKI_ASSERT(info.m_mipmap < tex.m_mipsCount);

	const GLenum target = GL_FRAMEBUFFER;
	switch(tex.m_target)
	{
	case GL_TEXTURE_2D:
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_TEXTURE_2D_MULTISAMPLE:
#endif
		ANKI_ASSERT(info.m_arrayIndex == 0);
		ANKI_ASSERT(info.m_depth == 0);
		ANKI_ASSERT(info.m_faceIndex == 0);

		glFramebufferTexture2D(
			target, attachment, tex.m_target, tex.getGlName(), info.m_mipmap);
		break;
	case GL_TEXTURE_CUBE_MAP:
		ANKI_ASSERT(info.m_arrayIndex == 0);
		ANKI_ASSERT(info.m_depth == 0);
		ANKI_ASSERT(info.m_faceIndex < 6);

		glFramebufferTexture2D(target,
			attachment,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + info.m_faceIndex,
			tex.getGlName(),
			info.m_mipmap);
		break;
	case GL_TEXTURE_2D_ARRAY:
		ANKI_ASSERT(info.m_arrayIndex < tex.m_depth);
		ANKI_ASSERT(info.m_depth == 0);
		ANKI_ASSERT(info.m_faceIndex == 0);

		glFramebufferTextureLayer(target,
			attachment,
			tex.getGlName(),
			info.m_mipmap,
			info.m_arrayIndex);
		break;
	case GL_TEXTURE_3D:
		ANKI_ASSERT(info.m_arrayIndex == 0);
		ANKI_ASSERT(info.m_depth < tex.m_depth);
		ANKI_ASSERT(info.m_faceIndex == 0);

		glFramebufferTextureLayer(
			target, attachment, tex.getGlName(), info.m_mipmap, info.m_depth);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		ANKI_ASSERT(info.m_arrayIndex < tex.m_depth);
		ANKI_ASSERT(info.m_depth == 0);
		ANKI_ASSERT(info.m_faceIndex < 6);

		glFramebufferTextureLayer(target,
			attachment,
			tex.getGlName(),
			info.m_mipmap,
			info.m_arrayIndex * 6 + info.m_faceIndex);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

//==============================================================================
void FramebufferImpl::bind(const GlState& state)
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
		if(m_in.m_colorAttachmentsCount)
		{
			glDrawBuffers(m_in.m_colorAttachmentsCount, &m_drawBuffers[0]);
		}

		// Invalidate
		if(m_invalidateBuffersCount)
		{
			glInvalidateFramebuffer(GL_FRAMEBUFFER,
				m_invalidateBuffersCount,
				&m_invalidateBuffers[0]);
		}

		// Clear buffers
		for(U i = 0; i < m_in.m_colorAttachmentsCount; i++)
		{
			const Attachment& att = m_in.m_colorAttachments[i];

			if(att.m_loadOperation == AttachmentLoadOperation::CLEAR)
			{
				// Enable write mask in case a pipeline changed it (else no
				// clear will happen) and then restore state
				Bool restore = false;
				if(state.m_colorWriteMasks[i][0] != true
					|| state.m_colorWriteMasks[i][1] != true
					|| state.m_colorWriteMasks[i][2] != true
					|| state.m_colorWriteMasks[i][3] != true)
				{
					glColorMaski(i, true, true, true, true);
					restore = true;
				}

				glClearBufferfv(GL_COLOR, i, &att.m_clearValue.m_colorf[0]);

				if(restore)
				{
					glColorMaski(i,
						state.m_colorWriteMasks[i][0],
						state.m_colorWriteMasks[i][1],
						state.m_colorWriteMasks[i][2],
						state.m_colorWriteMasks[i][3]);
				}
			}
		}

		if(m_in.m_depthStencilAttachment.m_texture.isCreated()
			&& m_in.m_depthStencilAttachment.m_loadOperation
				== AttachmentLoadOperation::CLEAR)
		{
			// Enable write mask in case a pipeline changed it (else no
			// clear will happen) and then restore state
			if(state.m_depthWriteMask == false)
			{
				glDepthMask(true);
			}

			glClearBufferfv(GL_DEPTH,
				0,
				&m_in.m_depthStencilAttachment.m_clearValue.m_depthStencil
					 .m_depth);

			if(state.m_depthWriteMask == false)
			{
				glDepthMask(false);
			}
		}
	}
}

} // end namespace anki
