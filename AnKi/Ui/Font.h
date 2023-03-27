// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui/UiObject.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Util/ClassWrapper.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup ui
/// @{

/// Font class.
class Font : public UiObject
{
public:
	Font() = default;

	~Font();

	/// Initialize the font.
	Error init(const CString& filename, ConstWeakArray<U32> fontHeights);

	/// Get font image atlas.
	ANKI_INTERNAL const TextureViewPtr& getTextureView() const
	{
		return m_texView;
	}

	ANKI_INTERNAL const ImFont& getImFont(U32 fontHeight) const
	{
		for(const FontEntry& f : m_fonts)
		{
			if(f.m_height == fontHeight)
			{
				return *f.m_imFont;
			}
		}

		ANKI_ASSERT(0);
		return *m_fonts[0].m_imFont;
	}

	ANKI_INTERNAL const ImFont& getFirstImFont() const
	{
		return *m_fonts[0].m_imFont;
	}

	ANKI_INTERNAL ImFontAtlas* getImFontAtlas()
	{
		return m_imFontAtlas.get();
	}

private:
	ClassWrapper<ImFontAtlas> m_imFontAtlas;
	UiDynamicArray<U8> m_fontData;

	class FontEntry
	{
	public:
		ImFont* m_imFont;
		U32 m_height;
	};

	UiDynamicArray<FontEntry> m_fonts;

	TexturePtr m_tex;
	TextureViewPtr m_texView; ///< Whole texture view

	void createTexture(const void* data, U32 width, U32 height);
};
/// @}

} // end namespace anki
