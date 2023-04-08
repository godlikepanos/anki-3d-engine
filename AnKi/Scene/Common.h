// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/String.h>
#include <AnKi/Scene/Forward.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>
#include <functional>

namespace anki {

// Forward
class ResourceManager;
class Input;
class UiManager;
class UnifiedGeometryMemoryPool;
class GpuSceneMemoryPool;
class GpuSceneMicroPatcher;
class ScriptManager;
class GrManager;
class PhysicsWorld;

/// @addtogroup scene
/// @{

#define ANKI_SCENE_LOGI(...) ANKI_LOG("SCEN", kNormal, __VA_ARGS__)
#define ANKI_SCENE_LOGE(...) ANKI_LOG("SCEN", kError, __VA_ARGS__)
#define ANKI_SCENE_LOGW(...) ANKI_LOG("SCEN", kWarning, __VA_ARGS__)
#define ANKI_SCENE_LOGF(...) ANKI_LOG("SCEN", kFatal, __VA_ARGS__)

class SceneMemoryPool : public HeapMemoryPool, public MakeSingleton<SceneMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	SceneMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "SceneMemPool")
	{
	}

	~SceneMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Scene, SceneMemoryPool)

#define ANKI_SCENE_ASSERT(expression) \
	std::invoke([&]() -> Bool { \
		const Bool ok = (expression); \
		if(!ok) [[unlikely]] \
		{ \
			ANKI_SCENE_LOGE("Expression failed: " #expression); \
		} \
		return ok; \
	})
/// @}

} // end namespace anki
