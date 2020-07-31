// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Framebuffer.h>
#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Framebuffer implementation.
class FramebufferImpl final : public Framebuffer, public GlObject
{
public:
	FramebufferImpl(GrManager* manager, CString name)
		: Framebuffer(manager, name)
	{
	}

	~FramebufferImpl()
	{
		destroyDeferred(getManager(), glDeleteFramebuffers);
	}

	/// Set all the attachments. It will overwrite the previous state. If the initalizer list is empty the it will bind
	/// the default framebuffer
	ANKI_USE_RESULT Error init(const FramebufferInitInfo& init);

	/// Bind it to the state. Call it in rendering thread
	void bind(const GlState& state, U32 minx, U32 miny, U32 width, U32 height) const;

	void endRenderPass() const;

	U getColorBufferCount() const
	{
		return m_in.m_colorAttachmentCount;
	}

private:
	FramebufferInitInfo m_in;

	Array<U32, 2> m_fbSize = {};
	Array<GLenum, MAX_COLOR_ATTACHMENTS> m_drawBuffers;
	Array<GLenum, MAX_COLOR_ATTACHMENTS + 1> m_invalidateBuffers;
	U8 m_invalidateBuffersCount = 0;
	Bool m_clearDepth = false;
	Bool m_clearStencil = false;

	/// Attach a texture
	static void attachTextureInternal(GLenum attachment, const TextureViewImpl& view,
									  const FramebufferAttachmentInfo& info);

	/// Create the FBO
	ANKI_USE_RESULT Error createFbo(const Array<U, MAX_COLOR_ATTACHMENTS + 1>& layers, GLenum depthStencilBindingPoint);
};
/// @}

} // end namespace anki
