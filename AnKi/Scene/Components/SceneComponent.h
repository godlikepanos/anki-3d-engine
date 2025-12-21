// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Scene/SceneSerializer.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/BitMask.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Core/Common.h>

namespace anki {

enum class SceneComponentType : U8
{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) k##name,
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(SceneComponentType)

enum class SceneComponentTypeMask : U32
{
	kNone = 0,

#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) k##name = 1 << U32(SceneComponentType::k##name),
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(SceneComponentTypeMask)

class SceneComponentType2
{
public:
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	static constexpr SceneComponentType k##name##Component = SceneComponentType::k##name;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
};

#define ANKI_SCENE_COMPONENT(className) \
public: \
	static constexpr SceneComponentType kClassType = SceneComponentType2::k##className; \
\
private:

class SceneComponentTypeInfo
{
public:
	const Char* m_name;
	F32 m_weight;
	Bool m_sceneNodeCanHaveMany;
	Bool m_serializable;
};

// Component names
inline Array<SceneComponentTypeInfo, U32(SceneComponentType::kCount)> kSceneComponentTypeInfos = {{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	{ \
		ANKI_STRINGIZE(name), weight, sceneNodeCanHaveMany, serializable \
	}
#define ANKI_SCENE_COMPONENT_SEPARATOR ,
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
}};

// Passed to SceneComponent::update.
class SceneComponentUpdateInfo
{
	friend class SceneGraph;

public:
	SceneNode* m_node = nullptr;
	const Second m_previousTime;
	const Second m_currentTime;
	const Second m_dt;
	const Bool m_forceUpdateSceneBounds;
	const Bool m_checkForResourceUpdates;
	StackMemoryPool* m_framePool = nullptr;

	SceneComponentUpdateInfo(Second prevTime, Second crntTime, Bool forceUpdateSceneBounds, Bool checkForResourceUpdates)
		: m_previousTime(prevTime)
		, m_currentTime(crntTime)
		, m_dt(crntTime - prevTime)
		, m_forceUpdateSceneBounds(forceUpdateSceneBounds)
		, m_checkForResourceUpdates(checkForResourceUpdates)
	{
	}

	void updateSceneBounds(Vec3 aabbMin, Vec3 aabbMax)
	{
		ANKI_ASSERT(aabbMin <= aabbMax);
		m_sceneMin = m_sceneMin.min(aabbMin);
		m_sceneMax = m_sceneMax.max(aabbMax);
	}

private:
	Vec3 m_sceneMin = Vec3(kMaxF32);
	Vec3 m_sceneMax = Vec3(kMinF32);
};

// Scene node component.
class SceneComponent
{
public:
	SceneComponent(SceneNode* node, SceneComponentType type, U32 uuid);

	virtual ~SceneComponent() = default;

	SceneComponentType getType() const
	{
		return SceneComponentType(m_type);
	}

	Timestamp getTimestamp() const
	{
		return m_timestamp;
	}

	U32 getUuid() const
	{
		return m_uuid;
	}

	ANKI_INTERNAL U32 getArrayIndex() const
	{
		ANKI_ASSERT(m_arrayIdx != kMaxU32 >> 8u);
		return m_arrayIdx;
	}

	ANKI_INTERNAL void setArrayIndex(U32 idx)
	{
		m_arrayIdx = idx & (kMaxU32 >> 8u);
		ANKI_ASSERT(m_arrayIdx == idx);
	}

	ANKI_INTERNAL virtual void onDestroy([[maybe_unused]] SceneNode& node)
	{
	}

	ANKI_INTERNAL virtual void update(SceneComponentUpdateInfo& info, Bool& updated) = 0;

	ANKI_INTERNAL virtual void onOtherComponentRemovedOrAdded([[maybe_unused]] SceneComponent* other, [[maybe_unused]] Bool added)
	{
	}

	// Don't call it directly.
	ANKI_INTERNAL void setTimestamp(Timestamp timestamp)
	{
		ANKI_ASSERT(timestamp > 0);
		ANKI_ASSERT(timestamp >= m_timestamp);
		m_timestamp = timestamp;
	}

	static constexpr F32 getUpdateOrderWeight(SceneComponentType type)
	{
		return kSceneComponentTypeInfos[type].m_weight;
	}

	Bool updatedThisFrame() const
	{
		return m_timestamp == GlobalFrameIndex::getSingleton().m_value;
	}

	virtual Error serialize([[maybe_unused]] SceneSerializer& serializer)
	{
		ANKI_ASSERT(!"Not supported");
		return Error::kNone;
	}

	void setSerialization(Bool enable)
	{
		ANKI_ASSERT(!enable || kSceneComponentTypeInfos[m_type].m_serializable);
		m_serialize = enable;
	}

	Bool getSerialization() const
	{
		return m_serialize;
	}

protected:
	// A convenience function for components to keep tabs on other components of a SceneNode
	template<typename TComponent>
	static void bookkeepComponent(SceneDynamicArray<TComponent*>& arr, SceneComponent* other, Bool added, Bool& firstDirty)
	{
		ANKI_ASSERT(other);
		ANKI_ASSERT(other->getType() == TComponent::kClassType);
		if(added)
		{
			for(auto it = arr.getBegin(); it != arr.getEnd(); ++it)
			{
				ANKI_ASSERT(*it != other);
			}

			arr.emplaceBack(static_cast<TComponent*>(other));
			firstDirty = arr.getSize() == 1;
		}
		else
		{
			[[maybe_unused]] Bool found = false;
			for(auto it = arr.getBegin(); it != arr.getEnd(); ++it)
			{
				if(*it == other)
				{
					firstDirty = it == arr.getBegin();
					arr.erase(it);
					found = true;
					break;
				}
			}
			ANKI_ASSERT(found);
		}
	}

	// A convenience function for components to keep tabs on other components of a SceneNode
	template<typename TComponent>
	static void bookkeepComponent(TComponent*& ptr, SceneComponent* other, Bool added)
	{
		ANKI_ASSERT(other);
		ANKI_ASSERT(other->getType() == TComponent::kClassType);
		if(added)
		{
			ANKI_ASSERT(ptr == nullptr);
			ptr = static_cast<TComponent*>(other);
		}
		else
		{
			ANKI_ASSERT(ptr == other);
			ptr = nullptr;
		}
	}

private:
	Timestamp m_timestamp = 1; // Indicates when an update happened
	U32 m_uuid : 31 = 0;
	U32 m_serialize : 1 = kSceneComponentTypeInfos[m_type].m_serializable;

	U32 m_arrayIdx : 24 = kMaxU32 >> 8u;
	U32 m_type : 8 = 0; // Cache the type ID.
};

} // end namespace anki
