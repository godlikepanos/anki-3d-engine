// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>
#include <anki/gr/Framebuffer.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Framebuffer implementation.
class FramebufferImpl : public GlObject
{
public:
	FramebufferImpl(GrManager* manager)
		: GlObject(manager)
	{
	}

	~FramebufferImpl()
	{
		destroyDeferred(glDeleteFramebuffers);
	}

	/// Set all the attachments. It will overwrite the previous state. If the initalizer list is empty the it will bind
	/// the default framebuffer
	ANKI_USE_RESULT Error init(const FramebufferInitInfo& init);

	/// Bind it to the state. Call it in rendering thread
	void bind(const GlState& state);

	U getColorBufferCount() const
	{
		return m_in.m_colorAttachmentCount;
	}

	Bool hasDepthBuffer() const
	{
		return m_in.m_depthStencilAttachment.m_texture.isCreated()
			&& !!(m_in.m_depthStencilAttachment.m_aspect & DepthStencilAspectBit::DEPTH);
	}

	Bool hasStencilBuffer() const
	{
		return m_in.m_depthStencilAttachment.m_texture.isCreated()
			&& !!(m_in.m_depthStencilAttachment.m_aspect & DepthStencilAspectBit::STENCIL);
	}

private:
	FramebufferInitInfo m_in;

	Array<GLenum, MAX_COLOR_ATTACHMENTS> m_drawBuffers;
	Array<GLenum, MAX_COLOR_ATTACHMENTS + 1> m_invalidateBuffers;
	U8 m_invalidateBuffersCount = 0;
	Bool8 m_bindDefault = false;
	DepthStencilAspectBit m_dsAspect = DepthStencilAspectBit::NONE;

	/// Attach a texture
	static void attachTextureInternal(GLenum attachment, const TextureImpl& tex, const FramebufferAttachmentInfo& info);

	/// Create the FBO
	ANKI_USE_RESULT Error createFbo(const Array<U, MAX_COLOR_ATTACHMENTS + 1>& layers, GLenum depthStencilBindingPoint);
};
/// @}

} // end namespace anki
