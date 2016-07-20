// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	FramebufferAttachmentInfo() = default;

	FramebufferAttachmentInfo(const FramebufferAttachmentInfo& b)
	{
		operator=(b);
	}

	~FramebufferAttachmentInfo() = default;

	FramebufferAttachmentInfo& operator=(const FramebufferAttachmentInfo& b)
	{
		m_texture = b.m_texture;
		m_surface = b.m_surface;
		m_loadOperation = b.m_loadOperation;
		m_storeOperation = b.m_storeOperation;
		memcpy(&m_clearValue, &b.m_clearValue, sizeof(m_clearValue));
		return *this;
	}
};

/// Framebuffer initializer. If you require the default framebuffer then set
/// m_colorAttachmentCount to 1 and don't set a color texture.
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
		return m_colorAttachmentCount == 1
			&& !m_colorAttachments[0].m_texture.isCreated();
	}

	Bool isValid() const
	{
		return m_colorAttachmentCount != 0
			|| m_depthStencilAttachment.m_texture.isCreated();
	}
};

/// GPU framebuffer.
class Framebuffer : public GrObject
{
public:
	static const GrObjectType CLASS_TYPE = GrObjectType::FRAMEBUFFER;

	/// Construct.
	Framebuffer(GrManager* manager, U64 hash = 0);

	/// Destroy.
	~Framebuffer();

	/// Access the implementation.
	FramebufferImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create.
	void init(const FramebufferInitInfo& init);

private:
	UniquePtr<FramebufferImpl> m_impl;
};
/// @}

} // end namespace anki
