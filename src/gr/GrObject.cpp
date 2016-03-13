// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrObject.h>
#include <anki/gr/GrManager.h>

namespace anki
{

//==============================================================================
GrObject::GrObject(GrManager* manager)
	: m_refcount(0)
	, m_manager(manager)
	, m_uuid(m_manager->getUuidIndex()++)
{
}

//==============================================================================
GrAllocator<U8> GrObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki
