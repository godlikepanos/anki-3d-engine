// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/core/Timestamp.h>
#include <anki/util/Functions.h>
#include <anki/util/BitMask.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Scene node component
class SceneComponent
{
public:
	// The type of the components
	enum class Type : U16
	{
		NONE,
		FRUSTUM,
		MOVE,
		RENDER,
		SPATIAL,
		LIGHT,
		LENS_FLARE,
		BODY,
		SECTOR_PORTAL,
		REFLECTION_PROBE,
		REFLECTION_PROXY,
		OCCLUDER,
		PLAYER_CONTROLLER,
		LAST_COMPONENT_ID = PLAYER_CONTROLLER
	};

	/// Construct the scene component.
	SceneComponent(Type type, SceneNode* node)
		: m_node(node)
		, m_type(type)
	{
	}

	virtual ~SceneComponent()
	{
	}

	Type getType() const
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
	virtual ANKI_USE_RESULT Error update(
		SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated)
	{
		updated = false;
		return ErrorCode::NONE;
	}

	/// Called if SceneComponent::update returned true.
	virtual ANKI_USE_RESULT Error onUpdate(
		SceneNode& node, F32 prevTime, F32 crntTime)
	{
		return ErrorCode::NONE;
	}

	/// Called only by the SceneGraph
	ANKI_USE_RESULT Error updateReal(
		SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated);

	void setAutomaticCleanup(Bool enable)
	{
		m_flags.set(AUTOMATIC_CLEANUP, enable);
	}

	Bool getAutomaticCleanup() const
	{
		return m_flags.get(AUTOMATIC_CLEANUP);
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

	SceneGraph& getSceneGraph();
	const SceneGraph& getSceneGraph() const;

protected:
	SceneNode* m_node = nullptr;
	Timestamp m_timestamp; ///< Indicates when an update happened

private:
	enum Flags
	{
		AUTOMATIC_CLEANUP = 1 << 0
	};

	Type m_type;
	BitMask<U8> m_flags;
};
/// @}

} // end namespace anki
