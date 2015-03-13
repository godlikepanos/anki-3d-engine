// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_FRAMEBUFFER_IMPL_H
#define ANKI_GR_GL_FRAMEBUFFER_IMPL_H

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/TextureHandle.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Framebuffer
class FramebufferImpl: public GlObject
{
public:
	using Base = GlObject;

	static const U32 MAX_COLOR_ATTACHMENTS = 4;

	/// Used to set the attachments
	class Attachment
	{
	public:
		Attachment()
		{}

		Attachment(const TextureHandle& tex, GLenum point, U32 layer = 0)
		:	m_tex(tex), 
			m_point(point), 
			m_layer(layer)
		{}

		TextureHandle m_tex;
		GLenum m_point;
		U32 m_layer;
	};

	FramebufferImpl() = default;

	~FramebufferImpl()
	{
		destroy();
	}

	/// Set all the attachments. It will overwrite the previous state. If the
	/// initalizer list is empty the it will bind the default framebuffer
	ANKI_USE_RESULT Error create(
		Attachment* attachmentsBegin, Attachment* attachmentsEnd);

	/// Bind it to the state. Call it in rendering thread
	/// @param invalidate If true invalidate the FB after binding it
	void bind(Bool invalidate);

	/// Blit another framebuffer to this
	void blit(const FramebufferImpl& fb, const Array<U32, 4>& sourceRect,
		const Array<U32, 4>& destRect, GLbitfield attachmentMask, Bool linear);

private:
	Array<TextureHandle, MAX_COLOR_ATTACHMENTS + 1> m_attachments;
	Bool8 m_bindDefault = false;

	/// Attach a texture
	static void attachTextureInternal(GLenum attachment, const TextureImpl& tex, 
		const U32 layer);

	/// Create the FBO
	ANKI_USE_RESULT Error createFbo(
		const Array<U, MAX_COLOR_ATTACHMENTS + 1>& layers,
		GLenum depthStencilBindingPoint);

	void destroy();
};
/// @}

} // end namespace anki

#endif
