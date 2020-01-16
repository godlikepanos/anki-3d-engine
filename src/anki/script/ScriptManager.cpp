// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/ScriptManager.h>
#include <anki/script/ScriptEnvironment.h>
#include <anki/util/Logger.h>

namespace anki
{

ScriptManager::ScriptManager()
{
}

ScriptManager::~ScriptManager()
{
	ANKI_SCRIPT_LOGI("Destroying scripting engine...");
}

Error ScriptManager::init(AllocAlignedCallback allocCb, void* allocCbData)
{
	ANKI_SCRIPT_LOGI("Initializing scripting engine...");

	m_alloc = ScriptAllocator(allocCb, allocCbData);

	ANKI_CHECK(m_lua.init(m_alloc, &m_otherSystems));

	return Error::NONE;
}

} // end namespace anki
