// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/UiManager.h>

namespace anki
{

UiManager::UiManager()
{
}

UiManager::~UiManager()
{
}

Error UiManager::init(HeapAllocator<U8> alloc, ResourceManager* resources, GrManager* gr)
{
	ANKI_ASSERT(resources);
	ANKI_ASSERT(gr);

	m_alloc = alloc;
	m_resources = resources;
	m_gr = gr;

	return ErrorCode::NONE;
}

} // end namespace anki