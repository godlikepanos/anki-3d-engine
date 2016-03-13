// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/util/Atomic.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Base of all graphics objects.
class GrObject : public NonCopyable
{
public:
	GrObject(GrManager* manager);

	virtual ~GrObject()
	{
	}

	GrManager& getManager()
	{
		return *m_manager;
	}

	const GrManager& getManager() const
	{
		return *m_manager;
	}

	GrAllocator<U8> getAllocator() const;

	Atomic<I32>& getRefcount()
	{
		return m_refcount;
	}

	/// A unique identifier for caching objects.
	U64 getUuid() const
	{
		return m_uuid;
	}

private:
	Atomic<I32> m_refcount;
	GrManager* m_manager;
	U64 m_uuid;
};
/// @}

} // end namespace anki
