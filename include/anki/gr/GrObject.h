// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GR_OBJECT_H
#define ANKI_GR_GR_OBJECT_H

#include "anki/gr/Common.h"
#include "anki/util/Atomic.h"

namespace anki {

/// @addtogroup graphics_private
/// @{

/// Base of all graphics objects.
class GrObject
{
public:
	GrObject(GrManager* manager)
	:	m_refcount(0),
		m_manager(manager)
	{}

	virtual ~GrObject()
	{}

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

private:
	Atomic<I32> m_refcount;
	GrManager* m_manager;
};
/// @}

} // end namespace anki

#endif

