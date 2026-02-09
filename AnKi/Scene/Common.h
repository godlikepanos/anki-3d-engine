// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/String.h>
#include <AnKi/Scene/Forward.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>

namespace anki {

// Forward
class ResourceManager;
class Input;
class UiManager;
class GpuSceneMicroPatcher;
class ScriptManager;
class GrManager;
class PhysicsWorld;

constexpr U32 kSceneUuidBits = 11;
constexpr U32 kSceneNodeUuidBits = 21;
constexpr U32 kSceneComponentUuidBits = kSceneNodeUuidBits;

#define ANKI_SCENE_LOGI(...) ANKI_LOG("SCEN", kNormal, __VA_ARGS__)
#define ANKI_SCENE_LOGE(...) ANKI_LOG("SCEN", kError, __VA_ARGS__)
#define ANKI_SCENE_LOGW(...) ANKI_LOG("SCEN", kWarning, __VA_ARGS__)
#define ANKI_SCENE_LOGF(...) ANKI_LOG("SCEN", kFatal, __VA_ARGS__)
#define ANKI_SCENE_LOGV(...) ANKI_LOG("SCEN", kVerbose, __VA_ARGS__)

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

#define ANKI_EXPECT(expression) \
	std::invoke([&]() -> Bool { \
		const Bool ok = !!(expression); \
		if(!ok) [[unlikely]] \
		{ \
			ANKI_SCENE_LOGE("Expression failed: " #expression); \
		} \
		return ok; \
	})

enum class GlobalRegistryRecordType : U32
{
	kSceneNode,

	kCount,
	kFirst = 0
};

class GlobalRegistryRecord : public IntrusiveListEnabled<GlobalRegistryRecord>
{
public:
	const Char* m_name;
	GlobalRegistryRecordType m_type;

	GlobalRegistryRecord(const Char* name, GlobalRegistryRecordType type);

	GlobalRegistryRecord(const GlobalRegistryRecord&) = delete;

	GlobalRegistryRecord& operator=(const GlobalRegistryRecord&) = delete;
};

// It's used to register global variables (like functions) uppon initialization. Then the records that can be retrieved using their name
class GlobalRegistry : public MakeSingletonLazyInit<GlobalRegistry>
{
public:
	void registerRecord(GlobalRegistryRecord* record)
	{
		ANKI_ASSERT(record && record->m_name);
		ANKI_ASSERT(tryFindRecord(record->m_name) == nullptr && "Already registered");
		m_records.pushBack(record);
	}

	GlobalRegistryRecord* findRecord(CString name)
	{
		GlobalRegistryRecord* rec = tryFindRecord(name);
		ANKI_ASSERT(rec && "Not found");
		return rec;
	}

	template<typename TFunc>
	FunctorContinue iterateRecords(TFunc func) const
	{
		FunctorContinue cont = FunctorContinue::kContinue;
		for(const GlobalRegistryRecord& it : m_records)
		{
			cont = func(it);
			if(cont == FunctorContinue::kStop)
			{
				break;
			}
		}
		return cont;
	}

private:
	IntrusiveList<GlobalRegistryRecord> m_records;

	GlobalRegistryRecord* tryFindRecord(CString name)
	{
		for(auto& it : m_records)
		{
			if(it.m_name == name)
			{
				return &it;
			}
		}

		return nullptr;
	}
};

inline GlobalRegistryRecord::GlobalRegistryRecord(const Char* name, GlobalRegistryRecordType type)
	: m_name(name)
	, m_type(type)
{
	ANKI_ASSERT(type < GlobalRegistryRecordType::kCount);
	GlobalRegistry::getSingleton().registerRecord(this);
}

} // end namespace anki
