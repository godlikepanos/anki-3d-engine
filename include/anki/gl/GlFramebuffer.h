// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_FRAMEBUFFER_H
#define ANKI_GL_GL_FRAMEBUFFER_H

#include "anki/gl/GlObject.h"
#include "anki/gl/GlTextureHandle.h"

namespace anki {

// Forward
class GlTexture;

/// @addtogroup opengl_private
/// @{

/// Framebuffer
class GlFramebuffer: public GlObject
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

		Attachment(const GlTextureHandle& tex, GLenum point, U32 layer = 0)
			: m_tex(tex), m_point(point), m_layer(layer)
		{}

		GlTextureHandle m_tex;
		GLenum m_point;
		U32 m_layer;
	};

	/// @name Constructors/Desctructor
	/// @{
	GlFramebuffer()
	{}

	/// Set all the attachments. It will overwrite the previous state. If the
	/// initalizer list is empty the it will bind the default framebuffer
	explicit GlFramebuffer(
		Attachment* attachmentsBegin, Attachment* attachmentsEnd);

	GlFramebuffer(GlFramebuffer&& b)
	{
		*this = std::move(b);
	}

	~GlFramebuffer()
	{
		destroy();
	}
	/// @}

	GlFramebuffer& operator=(GlFramebuffer&& b);

	/// Bind it to the state. Call it in rendering thread
	/// @param invalidate If true invalidate the FB after binding it
	void bind(Bool invalidate);

	/// Blit another framebuffer to this
	void blit(const GlFramebuffer& fb, const Array<U32, 4>& sourceRect,
		const Array<U32, 4>& destRect, GLbitfield attachmentMask, Bool linear);

private:
	Array<GlTextureHandle, MAX_COLOR_ATTACHMENTS + 1> m_attachments;
	Bool8 m_bindDefault = false;

	/// Attach a texture
	static void attachTextureInternal(GLenum attachment, const GlTexture& tex, 
		const U32 layer);

	/// Create the FBO
	void createFbo(const Array<U, MAX_COLOR_ATTACHMENTS + 1>& layers,
		GLenum depthStencilBindingPoint);

	void destroy();
};

/// @}

} // end namespace anki

#endif
