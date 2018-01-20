// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiObject.h>
#include <anki/gr/Texture.h>
#include <initializer_list>

namespace anki
{

/// @addtogroup ui
/// @{

/// Font class.
class Font : public UiObject
{
public:
	Font(UiManager* manager)
		: UiObject(manager)
	{
	}

	~Font();

	/// Initialize the font.
	ANKI_USE_RESULT Error init(const CString& filename, const std::initializer_list<U32>& fontHeights);

anki_internal:
	/// Get font image atlas.
	const TextureViewPtr& getTextureView() const
	{
		return m_texView;
	}

	const nk_user_font& getFont(U32 fontHeight) const
	{
		for(const NkFont& f : m_fonts)
		{
			if(f.m_height == fontHeight)
			{
				return f.m_font->handle;
			}
		}

		ANKI_ASSERT(0);
		return m_fonts[0].m_font->handle;
	}

private:
	nk_font_atlas m_atlas = {};

	struct NkFont
	{
		nk_font* m_font;
		U32 m_height;
	};

	DynamicArray<NkFont> m_fonts;

	TexturePtr m_tex;
	TextureViewPtr m_texView; ///< Whole texture view

	void createTexture(const void* data, U32 width, U32 height);
};
/// @}

} // end namespace anki
