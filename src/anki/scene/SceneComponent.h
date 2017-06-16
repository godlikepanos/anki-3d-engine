// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/core/Timestamp.h>
#include <anki/util/Functions.h>
#include <anki/util/BitMask.h>
#include <anki/util/List.h>

namespace anki
{

/// @addtogroup scene
/// @{

// The type of the components
enum class SceneComponentType : U16
{
	NONE,
	FRUSTUM,
	MOVE,
	RENDER,
	SPATIAL,
	LIGHT,
	LENS_FLARE,
	BODY,
	SECTOR,
	PORTAL,
	REFLECTION_PROBE,
	REFLECTION_PROXY,
	OCCLUDER,
	DECAL,
	PLAYER_CONTROLLER,

	COUNT,
	LAST_COMPONENT_ID = PLAYER_CONTROLLER
};

/// Scene node component
class SceneComponent : public IntrusiveListEnabled<SceneComponent>
{
public:
	/// Construct the scene component.
	SceneComponent(SceneComponentType type, SceneNode* node);

	virtual ~SceneComponent();

	SceneComponentType getType() const
	{
		return m_type;
	}

	Timestamp getTimestamp() const
	{
		return m_timestamp;
	}

	Timestamp getGlobalTimestamp() const;

	/// Do some updating
	/// @param[in,out] node Scene node of this component.
	/// @param prevTime Previous update time.
	/// @param crntTime Current update time.
	/// @param[out] updated true if an update happened.
	virtual ANKI_USE_RESULT Error update(SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated)
	{
		updated = false;
		return ErrorCode::NONE;
	}

	/// Called if SceneComponent::update returned true.
	virtual ANKI_USE_RESULT Error onUpdate(SceneNode& node, F32 prevTime, F32 crntTime)
	{
		return ErrorCode::NONE;
	}

	/// Called only by the SceneGraph
	ANKI_USE_RESULT Error updateReal(SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated);

	U64 getUuid() const
	{
		return m_uuid;
	}

	SceneNode& getSceneNode()
	{
		return *m_node;
	}

	const SceneNode& getSceneNode() const
	{
		return *m_node;
	}

	SceneAllocator<U8> getAllocator() const;

	SceneFrameAllocator<U8> getFrameAllocator() const;

	SceneGraph& getSceneGraph();

	const SceneGraph& getSceneGraph() const;

protected:
	SceneNode* m_node = nullptr;
	Timestamp m_timestamp = 0; ///< Indicates when an update happened

private:
	SceneComponentType m_type;
	U64 m_uuid;
};

/// Multiple lists of all types of components.
class SceneComponentLists : public NonCopyable
{
anki_internal:
	SceneComponentLists()
	{
	}

	~SceneComponentLists()
	{
	}

	void insertNew(SceneComponent* comp);

	void remove(SceneComponent* comp);

	template<typename TSceneComponentType, typename Func>
	void iterateComponents(Func func)
	{
		auto it = m_lists[TSceneComponentType::CLASS_TYPE].getBegin();
		auto end = m_lists[TSceneComponentType::CLASS_TYPE].getEnd();

		while(it != end)
		{
			func(static_cast<TSceneComponentType&>(*it));
			++it;
		}
	}

private:
	Array<IntrusiveList<SceneComponent>, U(SceneComponentType::COUNT)> m_lists;
};
/// @}

} // end namespace anki
