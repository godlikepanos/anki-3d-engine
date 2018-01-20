// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/Common.h>

namespace anki
{

/// @addtogroup ui
/// @{

/// The base of all UI objects.
class UiObject
{
public:
	UiObject(UiManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	virtual ~UiObject() = default;

	UiAllocator getAllocator() const;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

protected:
	UiManager* m_manager;
	Atomic<I32> m_refcount = {0};
};
/// @}

} // end namespace anki
