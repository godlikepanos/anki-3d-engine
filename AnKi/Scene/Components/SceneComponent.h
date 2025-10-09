// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/BitMask.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Core/Common.h>

namespace anki {

/// @addtogroup scene
/// @{

/// @memberof SceneComponent
enum class SceneComponentType : U8
{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, icon) k##name,
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(SceneComponentType)

/// @memberof SceneComponent
enum class SceneComponentTypeMask : U32
{
	kNone = 0,

#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, icon) k##name = 1 << U32(SceneComponentType::k##name),
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(SceneComponentTypeMask)

class SceneComponentType2
{
public:
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, icon) static constexpr SceneComponentType k##name##Component = SceneComponentType::k##name;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
};

#define ANKI_SCENE_COMPONENT(className) \
public: \
	static constexpr SceneComponentType kClassType = SceneComponentType2::k##className; \
\
private:

/// Component names
inline Array<const Char*, U32(SceneComponentType::kCount)> kSceneComponentTypeName = {
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, icon) ANKI_STRINGIZE(name)
#define ANKI_SCENE_COMPONENT_SEPARATOR ,
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
};

/// Passed to SceneComponent::update.
/// @memberof SceneComponent
class SceneComponentUpdateInfo
{
	friend class SceneGraph;

public:
	SceneNode* m_node = nullptr;
	const Second m_previousTime;
	const Second m_currentTime;
	const Second m_dt;
	const Bool m_forceUpdateSceneBounds;
	StackMemoryPool* m_framePool = nullptr;

	SceneComponentUpdateInfo(Second prevTime, Second crntTime, Bool forceUpdateSceneBounds)
		: m_previousTime(prevTime)
		, m_currentTime(crntTime)
		, m_dt(crntTime - prevTime)
		, m_forceUpdateSceneBounds(forceUpdateSceneBounds)
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

/// Scene node component.
class SceneComponent
{
public:
	/// Construct the scene component.
	SceneComponent(SceneNode* node, SceneComponentType type);

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

	/// Don't call it directly.
	ANKI_INTERNAL void setTimestamp(Timestamp timestamp)
	{
		ANKI_ASSERT(timestamp > 0);
		ANKI_ASSERT(timestamp >= m_timestamp);
		m_timestamp = timestamp;
	}

	static constexpr F32 getUpdateOrderWeight(SceneComponentType type)
	{
		return m_updateOrderWeights[type];
	}

	Bool updatedThisFrame() const
	{
		return m_timestamp == GlobalFrameIndex::getSingleton().m_value;
	}

protected:
	U32 regenerateUuid();

private:
	Timestamp m_timestamp = 1; ///< Indicates when an update happened
	U32 m_uuid = 0;

	U32 m_arrayIdx : 24 = kMaxU32 >> 8u;
	U32 m_type : 8 = 0; ///< Cache the type ID.

	static constexpr Array<F32, U32(SceneComponentType::kCount)> m_updateOrderWeights = {
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, icon) weight
#define ANKI_SCENE_COMPONENT_SEPARATOR ,
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
	};
};
/// @}

} // end namespace anki
