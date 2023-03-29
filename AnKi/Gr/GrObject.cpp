// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

GrObject::GrObject(GrObjectType type, CString name)
	: m_uuid(GrManager::getSingleton().getNewUuid())
	, m_refcount(0)
	, m_type(type)
{
	if(name.getLength() == 0)
	{
		name = "N/A";
	}

	m_name = static_cast<Char*>(GrMemoryPool::getSingleton().allocate(name.getLength() + 1, alignof(Char)));
	memcpy(m_name, &name[0], name.getLength() + 1);
}

GrObject::~GrObject()
{
	if(m_name)
	{
		GrMemoryPool::getSingleton().free(m_name);
	}
}

} // end namespace anki
