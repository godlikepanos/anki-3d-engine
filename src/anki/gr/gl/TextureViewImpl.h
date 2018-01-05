// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/TextureView.h>
#include <anki/gr/gl/GlObject.h>
#include <anki/gr/gl/TextureImpl.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Texture view implementation.
class TextureViewImpl final : public TextureView, public GlObject
{
public:
	MicroTextureView m_view = {};
	TexturePtr m_tex; ///< Hold a reference.

	TextureViewImpl(GrManager* manager)
		: TextureView(manager)
	{
	}

	~TextureViewImpl()
	{
	}

	void preInit(const TextureViewInitInfo& inf);

	void init()
	{
		m_view = static_cast<const TextureImpl&>(*m_tex).getOrCreateView(getSubresource());
	}

	TextureSubresourceInfo getSubresource() const
	{
		TextureSubresourceInfo out;
		out.m_baseMipmap = m_baseMip;
		out.m_mipmapCount = m_mipCount;
		out.m_baseLayer = m_baseLayer;
		out.m_layerCount = m_layerCount;
		out.m_baseFace = m_baseFace;
		out.m_faceCount = m_faceCount;
		out.m_depthStencilAspect = m_aspect;
		return out;
	}
};
/// @}

} // end namespace anki