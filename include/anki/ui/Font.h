// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/ui/UiInterface.h"

namespace anki {

/// @addtogroup ui
/// @{

class FontCharInfo
{
public:
	Vec2 m_advance;
	Vec2 m_size;
	Vec2 m_offset;
};

/// Font class.
class Font
{
	friend class IntrusivePtr<Font>;

public:
	Font(UiInterface* interface)
		: m_interface(interface)
	{}

	virtual ~Font();

	ANKI_USE_RESULT Error init(const CString& filename, U32 fontHeight);

private:
	WeakPtr<UiInterface> m_interface;
	Atomic<I32> m_refcount = {0};
	UiImagePtr m_tex;

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

