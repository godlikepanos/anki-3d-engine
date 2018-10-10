// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/util/Functions.h>
#include <anki/util/BitMask.h>

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
	REFLECTION_PROBE,
	REFLECTION_PROXY,
	OCCLUDER,
	DECAL,
	SKIN,
	SCRIPT,
	JOINT,
	TRIGGER,
	PLAYER_CONTROLLER,

	COUNT,
	LAST_COMPONENT_ID = PLAYER_CONTROLLER
};

/// Scene node component
class SceneComponent
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
	/// @param prevTime Previous update time.
	/// @param crntTime Current update time.
	/// @param[out] updated true if an update happened.
	virtual ANKI_USE_RESULT Error update(Second prevTime, Second crntTime, Bool& updated)
	{
		updated = false;
		return Error::NONE;
	}

	/// Called only by the SceneGraph
	ANKI_USE_RESULT Error updateReal(Second prevTime, Second crntTime, Bool& updated);

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

	/// The position in the owner SceneNode.
	U getIndex() const
	{
		return m_idx;
	}

	SceneAllocator<U8> getAllocator() const;

	SceneFrameAllocator<U8> getFrameAllocator() const;

	SceneGraph& getSceneGraph();

	const SceneGraph& getSceneGraph() const;

protected:
	SceneNode* m_node = nullptr;
	Timestamp m_timestamp = 1; ///< Indicates when an update happened

private:
	U64 m_uuid;
	U32 m_idx;
	SceneComponentType m_type;
};
/// @}

} // end namespace anki
