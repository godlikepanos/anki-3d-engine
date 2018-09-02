// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/resource/TextureResource.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Texture atlas resource class.
///
/// XML format:
/// @code
/// <textureAtlas>
/// 	<texture>path/to/tex.ankitex</texture>
/// 	<subTextureMargin>N</subTextureMargin>
/// 	<subTextures>
/// 		<subTexture>
/// 			<name>name</name>
/// 			<uv>0.1 0.2 0.5 0.6</uv>
/// 		</subTexture>
/// 		<subTexture>...</subTexture>
/// 		...
/// 	</subTextures>
/// </textureAtlas>
/// @endcode
class TextureAtlasResource : public ResourceObject
{
public:
	TextureAtlasResource(ResourceManager* manager);

	~TextureAtlasResource();

	/// Load a texture atlas.
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	TexturePtr getGrTexture() const
	{
		return m_tex->getGrTexture();
	}

	TextureViewPtr getGrTextureView() const
	{
		return m_tex->getGrTextureView();
	}

	U getWidth() const
	{
		return m_size[0];
	}

	U getHeight() const
	{
		return m_size[1];
	}

	U getSubTextureMargin() const
	{
		return m_margin;
	}

	/// Get the UV coordinates of a sub texture.
	ANKI_USE_RESULT Error getSubTextureInfo(CString name, F32 uv[4]) const;

private:
	class SubTex
	{
	public:
		CString m_name; ///< Points to TextureAtlas::m_subTexNames.
		Array<F32, 4> m_uv;
	};

	TextureResourcePtr m_tex;
	DynamicArray<char> m_subTexNames;
	DynamicArray<SubTex> m_subTexes;
	Array<U32, 2> m_size;
	U32 m_margin = 0;
};
/// @}

} // end namespace anki
