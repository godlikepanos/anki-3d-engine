// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Texture.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// TextureView init info.
class TextureViewInitInfo : public GrBaseInitInfo, public TextureSubresourceInfo
{
public:
	TexturePtr m_texture;

	TextureViewInitInfo(TexturePtr tex, CString name = {})
		: GrBaseInitInfo(name)
		, m_texture(tex)
	{
		m_firstMipmap = 0;
		m_mipmapCount = tex->getMipmapCount();
		m_firstLayer = 0;
		m_layerCount = tex->getLayerCount();
		m_firstFace = 0;
		m_faceCount =
			(tex->getTextureType() == TextureType::CUBE_ARRAY || tex->getTextureType() == TextureType::CUBE) ? 6 : 1;

		m_depthStencilAspect = computeFormatAspect(tex->getFormat());
	}

	TextureViewInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	TextureViewInitInfo(TexturePtr tex, const TextureSurfaceInfo& surf,
						DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE, CString name = {})
		: GrBaseInitInfo(name)
		, m_texture(tex)
	{
		m_firstMipmap = surf.m_level;
		m_mipmapCount = 1;
		m_firstLayer = surf.m_layer;
		m_layerCount = 1;
		m_firstFace = U8(surf.m_face);
		m_faceCount = 1;
		m_depthStencilAspect = aspect;
		ANKI_ASSERT(isValid());
	}

	TextureViewInitInfo(TexturePtr tex, const TextureSubresourceInfo& subresource, CString name = {})
		: GrBaseInitInfo(name)
		, m_texture(tex)
	{
		static_cast<TextureSubresourceInfo&>(*this) = subresource;
		ANKI_ASSERT(isValid());
	}

	Bool isValid() const
	{
		return m_texture.isCreated() && m_texture->isSubresourceValid(*this);
	}
};

/// GPU texture view.
class TextureView : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::TEXTURE_VIEW;

	TextureType getTextureType() const
	{
		ANKI_ASSERT(initialized());
		return m_texType;
	}

	const TextureSubresourceInfo& getSubresource() const
	{
		ANKI_ASSERT(initialized());
		return m_subresource;
	}

	/// Returns an index to be used for bindless access. Only for sampling.
	/// @note It's thread-safe
	U32 getOrCreateBindlessTextureIndex();

	/// Returns an index to be used for bindless access. For image read/write.
	/// @note It's thread-safe
	U32 getOrCreateBindlessImageIndex();

protected:
	TextureType m_texType = TextureType::COUNT;
	TextureSubresourceInfo m_subresource;

	/// Construct.
	TextureView(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
		m_subresource.m_depthStencilAspect = DepthStencilAspectBit::NONE;

		m_subresource.m_firstMipmap = MAX_U32;
		m_subresource.m_mipmapCount = MAX_U32;

		m_subresource.m_firstLayer = MAX_U32;
		m_subresource.m_layerCount = MAX_U32;

		m_subresource.m_firstFace = MAX_U8;
		m_subresource.m_faceCount = MAX_U8;
	}

	/// Destroy.
	~TextureView()
	{
	}

	Bool initialized() const
	{
		return m_texType != TextureType::COUNT && m_subresource.m_firstMipmap < MAX_U32
			   && m_subresource.m_mipmapCount < MAX_U32 && m_subresource.m_firstLayer < MAX_U32
			   && m_subresource.m_layerCount < MAX_U32 && m_subresource.m_firstFace < MAX_U8
			   && m_subresource.m_faceCount < MAX_U8;
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT TextureView* newInstance(GrManager* manager, const TextureViewInitInfo& init);
};
/// @}

} // end namespace anki
