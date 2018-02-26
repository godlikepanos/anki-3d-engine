// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/script/Common.h>

namespace anki
{

/// @addtogroup script
/// @{

/// The base of all script objects.
class ScriptObject
{
public:
	ScriptObject(ScriptManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	virtual ~ScriptObject()
	{
	}

	ScriptAllocator getAllocator() const;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

protected:
	ScriptManager* m_manager;

private:
	Atomic<I32> m_refcount = {0};
};
/// @}

} // end namespace anki