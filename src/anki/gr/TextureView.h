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
class TextureViewInitInfo : public GrBaseInitInfo
{
public:
	TexturePtr m_texture;

	U32 m_baseMipmap = MAX_U32;
	U32 m_mipmapCount = MAX_U32;

	U32 m_baseLayer = MAX_U32;
	U32 m_layerCount = MAX_U32;

	U8 m_baseFace = MAX_U8;
	U8 m_faceCount = MAX_U8;

	DepthStencilAspectBit m_depthStencilAspect = DepthStencilAspectBit::NONE;

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
		// TODO: m_depthStencilAspect = ?
	}

	TextureViewInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	Bool isValid() const
	{
#define ANKI_TEX_VIEW_ASSERT(x_) \
	if(!(x_))                    \
	{                            \
		return false;            \
	}
		ANKI_TEX_VIEW_ASSERT(m_texture.isCreated());
		const TextureType type = m_texture->getTextureType();
		const Bool cube = type == TextureType::CUBE_ARRAY || type == TextureType::CUBE;

		// Mips
		ANKI_TEX_VIEW_ASSERT(m_mipmapCount > 0);
		ANKI_TEX_VIEW_ASSERT(m_baseMipmap + m_mipmapCount <= m_texture->getMipmapCount());

		// Layers
		ANKI_TEX_VIEW_ASSERT(m_layerCount > 0);
		ANKI_TEX_VIEW_ASSERT(m_baseLayer + m_layerCount <= m_texture->getLayerCount());

		// Faces
		const U8 faceCount = (cube) ? 6 : 1;
		ANKI_TEX_VIEW_ASSERT(m_faceCount > 0);
		ANKI_TEX_VIEW_ASSERT(m_baseFace + m_faceCount <= faceCount);

		// Misc
		if(type == TextureType::CUBE_ARRAY && m_layerCount > 1)
		{
			ANKI_TEX_VIEW_ASSERT(m_faceCount == 6); // Because of the way surfaces are arranged in cube arrays
		}

#undef ANKI_TEX_VIEW_VALID
		return true;
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
