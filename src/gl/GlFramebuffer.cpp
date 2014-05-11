#include "anki/gl/GlFramebuffer.h"
#include "anki/gl/GlTexture.h"

namespace anki {

//==============================================================================
GlFramebuffer& GlFramebuffer::operator=(GlFramebuffer&& b)
{
	destroy();
	Base::operator=(std::forward<Base>(b));

	for(U i = 0; i < m_attachments.size(); i++)
	{
		m_attachments[i] = b.m_attachments[i];
		b.m_attachments[i] = GlTextureHandle();
	}

	m_bindDefault = b.m_bindDefault;
	b.m_bindDefault = false;

	return *this;
}

//==============================================================================
GlFramebuffer::GlFramebuffer(
	Attachment* attachmentsBegin, Attachment* attachmentsEnd)
{
	if(attachmentsBegin == nullptr || attachmentsEnd == nullptr)
	{
		m_bindDefault = true;
		return;
	}

	m_bindDefault = false;

	Array<U, MAX_COLOR_ATTACHMENTS + 1> layers;
	GLenum depthStencilBindingPoint = GL_NONE;
	Attachment* attachment = attachmentsBegin;
	do
	{
		if(attachment->m_point >= GL_COLOR_ATTACHMENT0 
			&& attachment->m_point < 
				GL_COLOR_ATTACHMENT0 + MAX_COLOR_ATTACHMENTS)
		{
			// Color attachment

			U32 i = attachment->m_point - GL_COLOR_ATTACHMENT0;
			m_attachments[i] = attachment->m_tex;
			layers[i] = attachment->m_layer;
		}
		else
		{
			// Depth & stencil

			m_attachments[MAX_COLOR_ATTACHMENTS] = attachment->m_tex;
			layers[MAX_COLOR_ATTACHMENTS] = attachment->m_layer;
			depthStencilBindingPoint = attachment->m_point;
		}

	} while(++attachment != attachmentsEnd);

	// Now create the FBO
	createFbo(layers, depthStencilBindingPoint);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//==============================================================================
void GlFramebuffer::destroy()
{
	if(m_glName != 0)
	{
		glDeleteFramebuffers(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
void GlFramebuffer::createFbo(
	const Array<U, MAX_COLOR_ATTACHMENTS + 1>& layers,
	GLenum depthStencilBindingPoint)
{
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
		
		const GlTexture& tex = m_attachments[i]._get();
		attachTextureInternal(GL_COLOR_ATTACHMENT0 + i, tex, layers[i]);
	}

	// Attach depth/stencil
	if(m_attachments[MAX_COLOR_ATTACHMENTS].isCreated())
	{
		ANKI_ASSERT(depthStencilBindingPoint != GL_NONE);

		const GlTexture& tex = m_attachments[MAX_COLOR_ATTACHMENTS]._get();
		attachTextureInternal(depthStencilBindingPoint, tex, 
			layers[MAX_COLOR_ATTACHMENTS]);
	}

	// Check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		destroy();
		throw ANKI_EXCEPTION("FBO is incomplete");
	}
}

//==============================================================================
void GlFramebuffer::attachTextureInternal(
	GLenum attachment, const GlTexture& tex, const U32 layer)
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
void GlFramebuffer::bind(Bool invalidate)
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
			static const Array<GLenum, GlFramebuffer::MAX_COLOR_ATTACHMENTS + 1>
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

} // end namespace anki

