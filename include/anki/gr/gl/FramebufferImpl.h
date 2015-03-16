// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_FRAMEBUFFER_IMPL_H
#define ANKI_GR_GL_FRAMEBUFFER_IMPL_H

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/TextureHandle.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Framebuffer implementation.
class FramebufferImpl: public GlObject
{
public:
	using Base = GlObject;
	using Initializer = FramebufferInitializer;

	FramebufferImpl(GrManager* manager)
	:	Base(manager)
	{}

	~FramebufferImpl()
	{
		destroy();
	}

	/// Set all the attachments. It will overwrite the previous state. If the
	/// initalizer list is empty the it will bind the default framebuffer
	ANKI_USE_RESULT Error create(Initializer& init);

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
