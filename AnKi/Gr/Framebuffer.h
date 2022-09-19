// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Gr/TextureView.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Framebuffer attachment info.
class FramebufferAttachmentInfo
{
public:
	TextureViewPtr m_textureView;

	AttachmentLoadOperation m_loadOperation = AttachmentLoadOperation::kClear;
	AttachmentStoreOperation m_storeOperation = AttachmentStoreOperation::kStore;

	AttachmentLoadOperation m_stencilLoadOperation = AttachmentLoadOperation::kClear;
	AttachmentStoreOperation m_stencilStoreOperation = AttachmentStoreOperation::kStore;

	ClearValue m_clearValue;
};

/// Framebuffer initializer.
class FramebufferInitInfo : public GrBaseInitInfo
{
public:
	Array<FramebufferAttachmentInfo, kMaxColorRenderTargets> m_colorAttachments;
	U32 m_colorAttachmentCount = 0;
	FramebufferAttachmentInfo m_depthStencilAttachment;

	class
	{
	public:
		TextureViewPtr m_textureView;
		U32 m_texelWidth = 0;
		U32 m_texelHeight = 0;
	} m_shadingRateImage;

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
		m_shadingRateImage = b.m_shadingRateImage;
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

		if(m_shadingRateImage.m_textureView)
		{
			if(m_shadingRateImage.m_texelHeight == 0 || m_shadingRateImage.m_texelWidth == 0)
			{
				return false;
			}

			if(!isPowerOfTwo(m_shadingRateImage.m_texelHeight) || !isPowerOfTwo(m_shadingRateImage.m_texelWidth))
			{
				return false;
			}
		}

		return true;
	}
};

/// GPU framebuffer.
class Framebuffer : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kFramebuffer;

protected:
	/// Construct.
	Framebuffer(GrManager* manager, CString name)
		: GrObject(manager, kClassType, name)
	{
	}

	/// Destroy.
	~Framebuffer()
	{
	}

private:
	[[nodiscard]] static Framebuffer* newInstance(GrManager* manager, const FramebufferInitInfo& init);
};
/// @}

} // end namespace anki
