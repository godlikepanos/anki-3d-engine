// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiObject.h>
#include <anki/gr/Texture.h>

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

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	/// Initialize the font.
	ANKI_USE_RESULT Error init(const CString& filename, U32 fontHeight);

anki_internal:
	/// Get font image atlas.
	const TexturePtr& getTexture() const
	{
		return m_tex;
	}

private:
	Atomic<I32> m_refcount = {0};

	nk_font_atlas m_atlas = {};
	nk_font* m_font = nullptr;

	TexturePtr m_tex;

	void createTexture(const void* data, U32 width, U32 height);
};

using FontPtr = IntrusivePtr<Font>;
/// @}

} // end namespace anki
