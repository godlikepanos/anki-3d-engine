// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
		m_baseMipmap = 0;
		m_mipmapCount = tex->getMipmapCount();
		m_baseLayer = 0;
		m_layerCount = tex->getLayerCount();
		m_baseFace = 0;
		m_faceCount =
			(tex->getTextureType() == TextureType::CUBE_ARRAY || tex->getTextureType() == TextureType::CUBE) ? 6 : 1;

		const PixelFormat fmt = tex->getPixelFormat();
		m_depthStencilAspect =
			(componentFormatIsDepth(fmt.m_components)) ? DepthStencilAspectBit::DEPTH : DepthStencilAspectBit::NONE;
		m_depthStencilAspect |=
			(componentFormatIsStencil(fmt.m_components)) ? DepthStencilAspectBit::STENCIL : DepthStencilAspectBit::NONE;
	}

	TextureViewInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	TextureViewInitInfo(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		DepthStencilAspectBit aspect = DepthStencilAspectBit::NONE,
		CString name = {})
		: GrBaseInitInfo(name)
		, m_texture(tex)
	{
		m_baseMipmap = surf.m_level;
		m_mipmapCount = 1;
		m_baseLayer = surf.m_layer;
		m_layerCount = 1;
		m_baseFace = surf.m_face;
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

	DepthStencilAspectBit getDepthStencilAspect() const
	{
		ANKI_ASSERT(initialized());
		return m_aspect;
	}

	Bool isSingleSurface() const
	{
		ANKI_ASSERT(initialized());
		ANKI_ASSERT(m_texType != TextureType::_3D);
		return m_mipCount == 1 && m_layerCount == 1 && m_faceCount == 1;
	}

	Bool isSingleVolume() const
	{
		ANKI_ASSERT(initialized());
		ANKI_ASSERT(m_texType == TextureType::_3D);
		return m_mipCount == 1;
	}

	U32 getBaseMipmap() const
	{
		ANKI_ASSERT(initialized());
		return m_baseMip;
	}

	U32 getMipmapCount() const
	{
		ANKI_ASSERT(initialized());
		return m_mipCount;
	}

	U32 getBaseLayer() const
	{
		ANKI_ASSERT(initialized());
		return m_baseLayer;
	}

	U32 getLayerCount() const
	{
		ANKI_ASSERT(initialized());
		return m_layerCount;
	}

	U32 getBaseFace() const
	{
		ANKI_ASSERT(initialized());
		return m_baseFace;
	}

	U32 getFaceCount() const
	{
		ANKI_ASSERT(initialized());
		return m_faceCount;
	}

protected:
	TextureType m_texType = TextureType::COUNT;
	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE;

	U32 m_baseMip = MAX_U32;
	U32 m_mipCount = MAX_U32;

	U32 m_baseLayer = MAX_U32;
	U32 m_layerCount = MAX_U32;

	U8 m_baseFace = MAX_U8;
	U8 m_faceCount = MAX_U8;

	/// Construct.
	TextureView(GrManager* manager)
		: GrObject(manager, CLASS_TYPE)
	{
	}

	/// Destroy.
	~TextureView()
	{
	}

	Bool initialized() const
	{
		return m_texType != TextureType::COUNT && m_baseMip < MAX_U32 && m_mipCount < MAX_U32 && m_baseLayer < MAX_U32
			&& m_layerCount < MAX_U32 && m_baseFace < MAX_U8 && m_faceCount < MAX_U8;
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT TextureView* newInstance(GrManager* manager, const TextureViewInitInfo& init);
};
/// @}

} // end namespace anki
