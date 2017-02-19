// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Texture.h>
#include <cstring>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Framebuffer attachment info.
class FramebufferAttachmentInfo
{
public:
	TexturePtr m_texture;
	TextureSurfaceInfo m_surface;
	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::STORE;
	ClearValue m_clearValue;

	AttachmentLoadOperation m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_stencilStoreOperation = AttachmentStoreOperation::STORE;

	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE; ///< Relevant only for depth stencil textures.
};

/// Framebuffer initializer. If you require the default framebuffer then set m_colorAttachmentCount to 1 and don't set a
/// color texture.
class FramebufferInitInfo
{
public:
	Array<FramebufferAttachmentInfo, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentCount = 0;
	FramebufferAttachmentInfo m_depthStencilAttachment;

	FramebufferInitInfo() = default;

	FramebufferInitInfo(const FramebufferInitInfo& b)
	{
		operator=(b);
	}

	~FramebufferInitInfo() = default;

	FramebufferInitInfo& operator=(const FramebufferInitInfo& b)
	{
		for(U i = 0; i < b.m_colorAttachmentCount; i++)
		{
			m_colorAttachments[i] = b.m_colorAttachments[i];
		}

		m_colorAttachmentCount = b.m_colorAttachmentCount;
		m_depthStencilAttachment = b.m_depthStencilAttachment;
		return *this;
	}

	Bool refersToDefaultFramebuffer() const
	{
		return m_colorAttachmentCount == 1 && !m_colorAttachments[0].m_texture.isCreated();
	}
};

/// GPU framebuffer.
class Framebuffer final : public GrObject
{
	ANKI_GR_OBJECT

anki_internal:
	UniquePtr<FramebufferImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::FRAMEBUFFER;

	/// Construct.
	Framebuffer(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~Framebuffer();

	/// Create.
	void init(const FramebufferInitInfo& init);
};
/// @}

} // end namespace anki
