// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ResourceObject.h"
#include "anki/resource/ResourceManager.h"

namespace anki {

//==============================================================================
ResourceObject::~ResourceObject()
{
	m_uuid.destroy(getAllocator());
}

//==============================================================================
ResourceAllocator<U8> ResourceObject::getAllocator() const
{
	return m_manager->getAllocator();
}

//==============================================================================
TempResourceAllocator<U8> ResourceObject::getTempAllocator() const
{
	return m_manager->getTempAllocator();
}

} // end namespace anki
