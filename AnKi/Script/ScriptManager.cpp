// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Script/ScriptEnvironment.h>
#include <AnKi/Util/Logger.h>

namespace anki {

ScriptManager::PoolInit::PoolInit(AllocAlignedCallback allocCb, void* allocCbData)
{
	ANKI_SCRIPT_LOGI("Initializing scripting engine...");

	ScriptMemoryPool::allocateSingleton(allocCb, allocCbData);
}

ScriptManager::PoolInit ::~PoolInit()
{
	ANKI_SCRIPT_LOGI("Destroying scripting engine...");

	ScriptMemoryPool::freeSingleton();
}

ScriptManager::ScriptManager(AllocAlignedCallback allocCb, void* allocCbData)
	: m_poolInit(allocCb, allocCbData)
{
	ANKI_SCRIPT_LOGI("Initializing scripting engine...");
}

ScriptManager::~ScriptManager()
{
}

} // end namespace anki
