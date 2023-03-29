// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/MemoryPool.h>

namespace anki {

// Forward
class LuaBinder;
class ScriptManager;
class ScriptEnvironment;

#define ANKI_SCRIPT_LOGI(...) ANKI_LOG("SCRI", kNormal, __VA_ARGS__)
#define ANKI_SCRIPT_LOGE(...) ANKI_LOG("SCRI", kError, __VA_ARGS__)
#define ANKI_SCRIPT_LOGW(...) ANKI_LOG("SCRI", kWarning, __VA_ARGS__)
#define ANKI_SCRIPT_LOGF(...) ANKI_LOG("SCRI", kFatal, __VA_ARGS__)

class ScriptMemoryPool : public HeapMemoryPool, public MakeSingleton<ScriptMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	ScriptMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "ScriptMemPool")
	{
	}

	~ScriptMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Script, ScriptMemoryPool)

} // end namespace anki
