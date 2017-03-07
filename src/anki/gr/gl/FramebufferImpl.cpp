// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

Error FramebufferImpl::init(const FramebufferInitInfo& init)
{
	ANKI_ASSERT(!isCreated());
	m_in = init;

	if(m_in.m_colorAttachmentCount == 1 && !m_in.m_colorAttachments[0].m_texture.isCreated())
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
	for(U i = 0; i < m_in.m_colorAttachmentCount; i++)
	{
		const FramebufferAttachmentInfo& att = m_in.m_colorAttachments[i];
		const GLenum binding = GL_COLOR_ATTACHMENT0 + i;

		attachTextureInternal(binding, *att.m_texture->m_impl, att);

		m_drawBuffers[i] = binding;

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}
	}

	// Attach depth/stencil
	if(m_in.m_depthStencilAttachment.m_texture.isCreated())
	{
		const FramebufferAttachmentInfo& att = m_in.m_depthStencilAttachment;
		const TextureImpl& tex = *att.m_texture->m_impl;

		GLenum binding;
		if(tex.m_format == GL_DEPTH_COMPONENT)
		{
			binding = GL_DEPTH_ATTACHMENT;
			m_dsAspect = DepthStencilAspectBit::DEPTH;
		}
		else if(tex.m_format == GL_STENCIL_INDEX)
		{
			binding = GL_STENCIL_ATTACHMENT;
			m_dsAspect = DepthStencilAspectBit::STENCIL;
		}
		else
		{
			ANKI_ASSERT(tex.m_format == GL_DEPTH_STENCIL);

			if(att.m_aspect == DepthStencilAspectBit::DEPTH)
			{
				binding = GL_DEPTH_ATTACHMENT;
			}
			else if(att.m_aspect == DepthStencilAspectBit::STENCIL)
			{
				binding = GL_STENCIL_ATTACHMENT;
			}
			else if(att.m_aspect == DepthStencilAspectBit::DEPTH_STENCIL)
			{
				binding = GL_DEPTH_STENCIL_ATTACHMENT;
			}
			else
			{
				ANKI_ASSERT(!"Need to set FramebufferAttachmentInfo::m_aspect");
				binding = 0;
			}

			m_dsAspect = att.m_aspect;
		}

		attachTextureInternal(binding, tex, att);

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}
	}

	// Check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		ANKI_GL_LOGE("FBO is incomplete. Status: 0x%x", status);
		return ErrorCode::FUNCTION_FAILED;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return ErrorCode::NONE;
}

void FramebufferImpl::attachTextureInternal(
	GLenum attachment, const TextureImpl& tex, const FramebufferAttachmentInfo& info)
{
	tex.checkSurfaceOrVolume(info.m_surface);

	const GLenum target = GL_FRAMEBUFFER;
	switch(tex.m_target)
	{
	case GL_TEXTURE_2D:
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_TEXTURE_2D_MULTISAMPLE:
#endif
		glFramebufferTexture2D(target, attachment, tex.m_target, tex.getGlName(), info.m_surface.m_level);
		break;
	case GL_TEXTURE_CUBE_MAP:
		glFramebufferTexture2D(target,
			attachment,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + info.m_surface.m_face,
			tex.getGlName(),
			info.m_surface.m_level);
		break;
	case GL_TEXTURE_2D_ARRAY:
		glFramebufferTextureLayer(target, attachment, tex.getGlName(), info.m_surface.m_level, info.m_surface.m_layer);
		break;
	case GL_TEXTURE_3D:
		glFramebufferTextureLayer(target, attachment, tex.getGlName(), info.m_surface.m_level, info.m_surface.m_depth);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glFramebufferTextureLayer(target,
			attachment,
			tex.getGlName(),
			info.m_surface.m_level,
			info.m_surface.m_layer * 6 + info.m_surface.m_face);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

void FramebufferImpl::bind(const GlState& state)
{
	if(m_bindDefault)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const FramebufferAttachmentInfo& att = m_in.m_colorAttachments[0];

		if(att.m_loadOperation == AttachmentLoadOperation::CLEAR)
		{
			glClearBufferfv(GL_COLOR, 0, &att.m_clearValue.m_colorf[0]);
		}
		else
		{
			/* For some reason the driver reports error
			ANKI_ASSERT(
				att.m_loadOperation == AttachmentLoadOperation::DONT_CARE);
			GLenum buff = GL_COLOR_ATTACHMENT0;
			glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &buff);*/
		}
	}
	else
	{
		ANKI_ASSERT(m_glName != 0);
		glBindFramebuffer(GL_FRAMEBUFFER, m_glName);

		// Set the draw buffers
		if(m_in.m_colorAttachmentCount)
		{
			glDrawBuffers(m_in.m_colorAttachmentCount, &m_drawBuffers[0]);
		}

		// Invalidate
		if(m_invalidateBuffersCount)
		{
			glInvalidateFramebuffer(GL_FRAMEBUFFER, m_invalidateBuffersCount, &m_invalidateBuffers[0]);
		}

		// Clear buffers
		for(U i = 0; i < m_in.m_colorAttachmentCount; i++)
		{
			const FramebufferAttachmentInfo& att = m_in.m_colorAttachments[i];

			if(att.m_loadOperation == AttachmentLoadOperation::CLEAR)
			{
				// Enable write mask in case a pipeline changed it (else no clear will happen) and then restore state
				Bool restore = false;
				if(state.m_colorWriteMasks[i][0] != true || state.m_colorWriteMasks[i][1] != true
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

		// Clear depth
		if(!!(m_dsAspect & DepthStencilAspectBit::DEPTH) && m_in.m_depthStencilAttachment.m_texture.isCreated()
			&& m_in.m_depthStencilAttachment.m_loadOperation == AttachmentLoadOperation::CLEAR)
		{
			// Enable write mask in case a pipeline changed it (else no clear will happen) and then restore state
			if(state.m_depthWriteMask == false)
			{
				glDepthMask(true);
			}

			glClearBufferfv(GL_DEPTH, 0, &m_in.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth);

			if(state.m_depthWriteMask == false)
			{
				glDepthMask(false);
			}
		}

		// Clear stencil
		if(!!(m_dsAspect & DepthStencilAspectBit::STENCIL) && m_in.m_depthStencilAttachment.m_texture.isCreated()
			&& m_in.m_depthStencilAttachment.m_stencilLoadOperation == AttachmentLoadOperation::CLEAR)
		{
			// Enable write mask in case a pipeline changed it (else no clear will happen) and then restore state
			// From the spec: The clear operation always uses the front stencil write mask when clearing the stencil
			// buffer
			if(state.m_stencilWriteMask[0] != MAX_U32)
			{
				glStencilMaskSeparate(GL_FRONT, MAX_U32);
			}

			GLint clearVal = m_in.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_stencil;
			glClearBufferiv(GL_STENCIL, 0, &clearVal);

			if(state.m_stencilWriteMask[0] != MAX_U32)
			{
				glStencilMaskSeparate(GL_FRONT, state.m_stencilWriteMask[0]);
			}
		}
	}
}

} // end namespace anki
