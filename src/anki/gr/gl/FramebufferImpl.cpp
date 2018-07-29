// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/TextureViewImpl.h>
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

	glGenFramebuffers(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);
	const GLenum target = GL_FRAMEBUFFER;
	glBindFramebuffer(target, m_glName);

	// Attach color
	for(U i = 0; i < m_in.m_colorAttachmentCount; i++)
	{
		const FramebufferAttachmentInfo& att = m_in.m_colorAttachments[i];
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*att.m_textureView);
		ANKI_ASSERT(viewImpl.m_tex->isSubresourceGoodForFramebufferAttachment(viewImpl.getSubresource()));

		const GLenum binding = GL_COLOR_ATTACHMENT0 + i;

		attachTextureInternal(binding, viewImpl, att);

		m_drawBuffers[i] = binding;

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}

		if(m_fbSize[0] == 0)
		{
			m_fbSize[0] = viewImpl.m_tex->getWidth() >> viewImpl.getSubresource().m_firstMipmap;
			m_fbSize[1] = viewImpl.m_tex->getHeight() >> viewImpl.getSubresource().m_firstMipmap;
		}
		else
		{
			ANKI_ASSERT(m_fbSize[0] == (viewImpl.m_tex->getWidth() >> viewImpl.getSubresource().m_firstMipmap));
			ANKI_ASSERT(m_fbSize[1] == (viewImpl.m_tex->getHeight() >> viewImpl.getSubresource().m_firstMipmap));
		}
	}

	// Attach depth/stencil
	if(m_in.m_depthStencilAttachment.m_textureView.isCreated())
	{
		const FramebufferAttachmentInfo& att = m_in.m_depthStencilAttachment;
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*att.m_textureView);
		ANKI_ASSERT(viewImpl.m_tex->isSubresourceGoodForFramebufferAttachment(viewImpl.getSubresource()));

		GLenum binding;
		if(viewImpl.getSubresource().m_depthStencilAspect == DepthStencilAspectBit::DEPTH)
		{
			binding = GL_DEPTH_ATTACHMENT;
		}
		else if(viewImpl.getSubresource().m_depthStencilAspect == DepthStencilAspectBit::STENCIL)
		{
			binding = GL_STENCIL_ATTACHMENT;
		}
		else
		{
			ANKI_ASSERT(viewImpl.getSubresource().m_depthStencilAspect == DepthStencilAspectBit::DEPTH_STENCIL);
			binding = GL_DEPTH_STENCIL_ATTACHMENT;
		}

		attachTextureInternal(binding, viewImpl, att);

		if(att.m_loadOperation == AttachmentLoadOperation::DONT_CARE)
		{
			m_invalidateBuffers[m_invalidateBuffersCount++] = binding;
		}

		if(m_fbSize[0] == 0)
		{
			m_fbSize[0] = viewImpl.m_tex->getWidth() >> viewImpl.getSubresource().m_firstMipmap;
			m_fbSize[1] = viewImpl.m_tex->getHeight() >> viewImpl.getSubresource().m_firstMipmap;
		}
		else
		{
			ANKI_ASSERT(m_fbSize[0] == (viewImpl.m_tex->getWidth() >> viewImpl.getSubresource().m_firstMipmap));
			ANKI_ASSERT(m_fbSize[1] == (viewImpl.m_tex->getHeight() >> viewImpl.getSubresource().m_firstMipmap));
		}

		// Misc
		m_clearDepth = !!(viewImpl.getSubresource().m_depthStencilAspect & DepthStencilAspectBit::DEPTH)
					   && att.m_loadOperation == AttachmentLoadOperation::CLEAR;

		m_clearStencil = !!(viewImpl.getSubresource().m_depthStencilAspect & DepthStencilAspectBit::STENCIL)
						 && att.m_stencilLoadOperation == AttachmentLoadOperation::CLEAR;
	}

	// Check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		ANKI_GL_LOGE("FBO is incomplete. Status: 0x%x", status);
		return Error::FUNCTION_FAILED;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return Error::NONE;
}

void FramebufferImpl::attachTextureInternal(
	GLenum attachment, const TextureViewImpl& view, const FramebufferAttachmentInfo& info)
{
	const GLenum target = GL_FRAMEBUFFER;
	const TextureImpl& tex = static_cast<const TextureImpl&>(*view.m_tex);

	switch(tex.m_target)
	{
	case GL_TEXTURE_2D:
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_TEXTURE_2D_MULTISAMPLE:
#endif
		glFramebufferTexture2D(target, attachment, tex.m_target, tex.getGlName(), view.getSubresource().m_firstMipmap);
		break;
	case GL_TEXTURE_CUBE_MAP:
		glFramebufferTexture2D(target,
			attachment,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + view.getSubresource().m_firstFace,
			tex.getGlName(),
			view.getSubresource().m_firstMipmap);
		break;
	case GL_TEXTURE_2D_ARRAY:
		glFramebufferTextureLayer(target,
			attachment,
			tex.getGlName(),
			view.getSubresource().m_firstMipmap,
			view.getSubresource().m_firstLayer);
		break;
	case GL_TEXTURE_3D:
		ANKI_ASSERT(!"TODO");
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glFramebufferTextureLayer(target,
			attachment,
			tex.getGlName(),
			view.getSubresource().m_firstMipmap,
			view.getSubresource().m_firstLayer * 6 + view.getSubresource().m_firstFace);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

void FramebufferImpl::bind(const GlState& state, U32 minx, U32 miny, U32 width, U32 height) const
{
	ANKI_ASSERT(width > 0 && height > 0);

	if(m_in.getName() && static_cast<const GrManagerImpl&>(getManager()).debugMarkersEnabled())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, m_glName, 0, &m_in.getName()[0]);
	}

	// Clear in the render area
	const U maxx = min<U>(minx + width, m_fbSize[0]);
	const U maxy = min<U>(miny + height, m_fbSize[1]);
	ANKI_ASSERT(minx < m_fbSize[0] && miny < m_fbSize[1] && maxx <= m_fbSize[0] && maxy <= m_fbSize[1]);
	width = maxx - minx;
	height = maxy - miny;
	glScissor(minx, miny, width, height);

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
		glInvalidateSubFramebuffer(
			GL_FRAMEBUFFER, m_invalidateBuffersCount, &m_invalidateBuffers[0], minx, miny, width, height);
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
				|| state.m_colorWriteMasks[i][2] != true || state.m_colorWriteMasks[i][3] != true)
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
	if(m_clearDepth)
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
	if(m_clearStencil)
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

	glScissor(state.m_scissor[0], state.m_scissor[1], state.m_scissor[2], state.m_scissor[3]);
}

void FramebufferImpl::endRenderPass() const
{
	if(m_in.getName() && static_cast<const GrManagerImpl&>(getManager()).debugMarkersEnabled())
	{
		glPopDebugGroup();
	}
}

} // end namespace anki
