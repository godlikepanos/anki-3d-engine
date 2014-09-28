// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SCENE_COMPONENT_H
#define ANKI_SCENE_SCENE_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/core/Timestamp.h"
#include "anki/util/Functions.h"

namespace anki {

/// Scene node component
class SceneComponent
{
public:
	// The type of the components
	enum class Type: U16
	{
		NONE,
		FRUSTUM,
		MOVE,
		RENDER,
		SPATIAL,
		LIGHT,
		INSTANCE,
		RIGID_BODY,
		LAST_COMPONENT_ID = RIGID_BODY
	};

	/// Construct the scene component.
	SceneComponent(Type type, SceneNode* node)
	:	m_type(type)
	{}

	Type getType() const
	{
		return m_type;
	}

	Timestamp getTimestamp() const
	{
		return m_timestamp;
	}

	/// Do some reseting before frame starts
	virtual void reset()
	{}

	/// Do some updating
	/// @return true if an update happened
	virtual Bool update(SceneNode& node, F32 prevTime, F32 crntTime)
	{
		return false;
	}

	/// Called if SceneComponent::update returned true.
	virtual void onUpdate(SceneNode& node, F32 prevTime, F32 crntTime)
	{}

	/// Called only by the SceneGraph
	Bool updateReal(SceneNode& node, F32 prevTime, F32 crntTime);

	template<typename TComponent>
	TComponent& downCast()
	{
		ANKI_ASSERT(TComponent::getClassType() == getType());
		TComponent* out = staticCastPtr<TComponent*>(this);
		return *out;
	}

protected:
	Timestamp m_timestamp; ///< Indicates when an update happened

private:
	Type m_type;
};

} // end namespace anki

#endif
