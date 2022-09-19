// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Gr/Texture.h>

namespace anki {

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
			(tex->getTextureType() == TextureType::kCubeArray || tex->getTextureType() == TextureType::kCube) ? 6 : 1;

		m_depthStencilAspect = getFormatInfo(tex->getFormat()).m_depthStencil;
	}

	TextureViewInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	TextureViewInitInfo(TexturePtr tex, const TextureSurfaceInfo& surf,
						DepthStencilAspectBit aspect = DepthStencilAspectBit::kNone, CString name = {})
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
	static constexpr GrObjectType kClassType = GrObjectType::kTextureView;

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

protected:
	TextureType m_texType = TextureType::kCount;
	TextureSubresourceInfo m_subresource;

	/// Construct.
	TextureView(GrManager* manager, CString name)
		: GrObject(manager, kClassType, name)
	{
		m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kNone;

		m_subresource.m_firstMipmap = kMaxU32;
		m_subresource.m_mipmapCount = kMaxU32;

		m_subresource.m_firstLayer = kMaxU32;
		m_subresource.m_layerCount = kMaxU32;

		m_subresource.m_firstFace = kMaxU8;
		m_subresource.m_faceCount = kMaxU8;
	}

	/// Destroy.
	~TextureView()
	{
	}

	Bool initialized() const
	{
		return m_texType != TextureType::kCount && m_subresource.m_firstMipmap < kMaxU32
			   && m_subresource.m_mipmapCount < kMaxU32 && m_subresource.m_firstLayer < kMaxU32
			   && m_subresource.m_layerCount < kMaxU32 && m_subresource.m_firstFace < kMaxU8
			   && m_subresource.m_faceCount < kMaxU8;
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static TextureView* newInstance(GrManager* manager, const TextureViewInitInfo& init);
};
/// @}

} // end namespace anki
