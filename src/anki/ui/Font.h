// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiInterface.h>

namespace anki
{

/// @addtogroup ui
/// @{

class FontCharInfo
{
public:
	UVec2 m_advance; ///< Char advance.
	Rect m_imageRect; ///< The rect inside the image atlas.
	IVec2 m_offset; ///< Left and top bearing.
};

/// Font class.
class Font
{
	friend class IntrusivePtr<Font>;
	friend IntrusivePtr<Font>::Deleter;

public:
	Font(UiInterface* interface)
		: m_interface(interface)
	{
	}

	~Font();

	/// Initialize the font.
	ANKI_USE_RESULT Error init(const CString& filename, U32 fontHeight);

anki_internal:
	/// Get info for a character.
	const FontCharInfo& getCharInfo(char c) const
	{
		ANKI_ASSERT(c >= FIRST_CHAR);
		return m_chars[c - ' '];
	}

	/// Get font image atlas.
	const UiImagePtr& getImage() const
	{
		return m_img;
	}

private:
	static const char FIRST_CHAR = ' ';
	static const char LAST_CHAR = 127;
	static const U CHAR_COUNT = LAST_CHAR - FIRST_CHAR + 1;

	WeakPtr<UiInterface> m_interface;
	Atomic<I32> m_refcount = {0};
	UiImagePtr m_img;
	Array<FontCharInfo, CHAR_COUNT> m_chars;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	UiAllocator getAllocator() const
	{
		return m_interface->getAllocator();
	}
};

using FontPtr = IntrusivePtr<Font>;
/// @}

} // end namespace anki
