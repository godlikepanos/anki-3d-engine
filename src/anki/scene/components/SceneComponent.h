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
	OCCLUDER,
	DECAL,
	SKIN,
	SCRIPT,
	JOINT,
	TRIGGER,
	FOG_DENSITY,
	PLAYER_CONTROLLER,

	COUNT,
	LAST_COMPONENT_ID = PLAYER_CONTROLLER
};

/// Scene node component
class SceneComponent
{
public:
	/// Construct the scene component.
	SceneComponent(SceneComponentType type)
		: m_type(type)
	{
	}

	virtual ~SceneComponent()
	{
	}

	SceneComponentType getType() const
	{
		return m_type;
	}

	Timestamp getTimestamp() const
	{
		return m_timestamp;
	}

	/// Do some updating
	/// @param node The owner node of this component.
	/// @param prevTime Previous update time.
	/// @param crntTime Current update time.
	/// @param[out] updated true if an update happened.
	virtual ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
	{
		updated = false;
		return Error::NONE;
	}

	/// Called only by the SceneGraph
	ANKI_USE_RESULT Error updateReal(SceneNode& node, Second prevTime, Second crntTime, Bool& updated);

	/// Don't call it.
	void setTimestamp(Timestamp timestamp)
	{
		ANKI_ASSERT(timestamp > 0);
		ANKI_ASSERT(timestamp >= m_timestamp);
		m_timestamp = timestamp;
	}

private:
	Timestamp m_timestamp = 1; ///< Indicates when an update happened
	SceneComponentType m_type;
};
/// @}

} // end namespace anki
