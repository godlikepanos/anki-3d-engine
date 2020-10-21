// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrObject.h>
#include <anki/gr/GrManager.h>

namespace anki
{

GrObject::GrObject(GrManager* manager, GrObjectType type, CString name)
	: m_manager(manager)
	, m_uuid(m_manager->getNewUuid())
	, m_refcount(0)
	, m_type(type)
{
	if(name.getLength() == 0)
	{
		name = "N/A";
	}

	m_name = static_cast<char*>(manager->getAllocator().getMemoryPool().allocate(name.getLength() + 1, alignof(char)));
	memcpy(const_cast<char*>(&m_name[0]), &name[0], name.getLength() + 1);
}

GrObject::~GrObject()
{
	if(m_name)
	{
		char* ptr = const_cast<char*>(&m_name[0]);
		m_manager->getAllocator().getMemoryPool().free(ptr);
	}
}

GrAllocator<U8> GrObject::getAllocator() const
{
	return m_manager->getAllocator();
}

} // end namespace anki
