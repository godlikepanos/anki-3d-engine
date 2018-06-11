// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrObject.h>
#include <anki/gr/GrManager.h>

namespace anki
{

GrObject::GrObject(GrManager* manager, GrObjectType type, CString name)
	: m_refcount(0)
	, m_manager(manager)
	, m_uuid(m_manager->getNewUuid())
	, m_type(type)
{
	if(name && name.getLength())
	{
		ANKI_ASSERT(name.getLength() <= MAX_GR_OBJECT_NAME_LENGTH);
		memcpy(&m_name[0], &name[0], name.getLength() + 1);
	}
	else
	{
		m_name[0] = 'N';
		m_name[1] = '/';
		m_name[2] = 'A';
		m_name[3] = '\0';
	}
}

GrObject::~GrObject()
{
}

GrAllocator<U8> GrObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki
