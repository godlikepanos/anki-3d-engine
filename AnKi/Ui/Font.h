// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

ANKI_CVAR(NumericCVar<U32>, Ui, GlobalFontBias, 0, 0, 100, "Bias that will be applied to all fonts")
ANKI_CVAR(NumericCVar<F32>, Ui, GlobalFontScale, 1.0f, 0.1f, 100.0f, "Scale that will be applied to all fonts")

/// Font class.
class Font : public UiObject
{
public:
	Font() = default;

	~Font();

	/// Initialize the font.
	Error init(const CString& filename, ConstWeakArray<U32> fontHeights);

	/// Get font image atlas.
	ANKI_INTERNAL const TexturePtr& getTexture() const
	{
		return m_tex;
	}

	ANKI_INTERNAL const ImFont& getImFont(U32 fontHeight) const
	{
		for(const FontEntry& f : m_fonts)
		{
			if(f.m_height == U32(F32(fontHeight) * g_cvarUiGlobalFontScale) + g_cvarUiGlobalFontBias)
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
	UiImageIdData m_imgData;

	void createTexture(const void* data, U32 width, U32 height);
};
/// @}

} // end namespace anki
