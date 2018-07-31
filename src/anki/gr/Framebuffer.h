// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/TextureView.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Framebuffer attachment info.
class FramebufferAttachmentInfo
{
public:
	TextureViewPtr m_textureView;

	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::STORE;

	AttachmentLoadOperation m_stencilLoadOperation = AttachmentLoadOperation::CLEAR;
	AttachmentStoreOperation m_stencilStoreOperation = AttachmentStoreOperation::STORE;

	ClearValue m_clearValue;
};

/// Framebuffer initializer.
class FramebufferInitInfo : public GrBaseInitInfo
{
public:
	Array<FramebufferAttachmentInfo, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
	U32 m_colorAttachmentCount = 0;
	FramebufferAttachmentInfo m_depthStencilAttachment;

	FramebufferInitInfo()
		: GrBaseInitInfo()
	{
	}

	FramebufferInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	FramebufferInitInfo(const FramebufferInitInfo& b)
		: GrBaseInitInfo(b)
	{
		operator=(b);
	}

	~FramebufferInitInfo() = default;

	FramebufferInitInfo& operator=(const FramebufferInitInfo& b)
	{
		GrBaseInitInfo::operator=(b);

		for(U i = 0; i < b.m_colorAttachmentCount; i++)
		{
			m_colorAttachments[i] = b.m_colorAttachments[i];
		}

		m_colorAttachmentCount = b.m_colorAttachmentCount;
		m_depthStencilAttachment = b.m_depthStencilAttachment;
		return *this;
	}

	Bool isValid() const
	{
		for(U i = 0; i < m_colorAttachmentCount; ++i)
		{
			if(!m_colorAttachments[i].m_textureView.isCreated())
			{
				return false;
			}
		}

		if(m_colorAttachmentCount == 0 && !m_depthStencilAttachment.m_textureView.isCreated())
		{
			return false;
		}

		return true;
	}
};

/// GPU framebuffer.
class Framebuffer : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::FRAMEBUFFER;

protected:
	/// Construct.
	Framebuffer(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~Framebuffer()
	{
	}

private:
	static ANKI_USE_RESULT Framebuffer* newInstance(GrManager* manager, const FramebufferInitInfo& init);
};
/// @}

} // end namespace anki
