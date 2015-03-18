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

struct Attachment
{
	TextureHandle m_texture;
	U32 m_layer = 0;
	U32 m_mipmap = 0;
};

/// Framebuffer initializer.
struct FramebufferInitializer
{
	Array<Attachment, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentsCount = 0;
	Attachment m_depthStencilAttachment;
};
/// @}

} // end namespace anki

#endif

