// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_FRAMEBUFFER_COMMON_H
#define ANKI_GR_FRAMEBUFFER_COMMON_H

#include "anki/gr/Common.h"
#include "anki/gr/TextureHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{
class Attachment
{
public:
	TextureHandle m_texture;
	U32 m_layer = 0;
	U32 m_mipmap = 0;
	PixelFormat m_format;
	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::STORE;
	union
	{
		Array<F32, 4> m_float = {{0.0, 0.0, 0.0, 0.0}};
		Array<I32, 4> m_int;
		Array<U32, 4> m_uint;
	} m_clearColor;

	Attachment() = default;

	Attachment(const Attachment& b)
	{
		operator=(b);
	}

	~Attachment() = default;

	Attachment& operator=(const Attachment& b)
	{
		m_texture = b.m_texture;
		m_layer = b.m_layer;
		m_mipmap = b.m_mipmap;
		m_format = b.m_format;
		m_loadOperation = b.m_loadOperation;
		m_storeOperation = b.m_storeOperation;
		memcpy(&m_clearColor, &b.m_clearColor, sizeof(m_clearColor));
		return *this;
	}
};

/// Framebuffer initializer.
class FramebufferInitializer
{
public:
	Array<Attachment, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentsCount = 0;
	Attachment m_depthStencilAttachment;

	FramebufferInitializer() = default;

	FramebufferInitializer(const FramebufferInitializer& b)
	{
		operator=(b);
	}

	~FramebufferInitializer() = default;

	FramebufferInitializer& operator=(const FramebufferInitializer& b)
	{
		for(U i = 0; i < b.m_colorAttachmentsCount; i++)
		{
			m_colorAttachments[i] = b.m_colorAttachments[i];
		}

		m_colorAttachmentsCount = b.m_colorAttachmentsCount;
		m_depthStencilAttachment = b.m_depthStencilAttachment;
		return *this;
	}
};
/// @}

} // end namespace anki

#endif

