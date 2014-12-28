// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SCENE_COMPONENT_H
#define ANKI_SCENE_SCENE_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/core/Timestamp.h"
#include "anki/util/Functions.h"
#include "anki/util/Bitset.h"

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
		LENS_FLARE,
		BODY,
		LAST_COMPONENT_ID = BODY
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
	/// @param[out] updated true if an update happened
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

	template<typename TComponent>
	TComponent& downCast()
	{
		ANKI_ASSERT(TComponent::getClassType() == getType());
		TComponent* out = staticCastPtr<TComponent*>(this);
		return *out;
	}

	void setAutomaticCleanup(Bool enable)
	{
		m_flags.enableBits(AUTOMATIC_CLEANUP, enable);	
	}

	Bool getAutomaticCleanup() const
	{
		return m_flags.bitsEnabled(AUTOMATIC_CLEANUP);
	}

protected:
	Timestamp m_timestamp; ///< Indicates when an update happened

private:
	enum Flags
	{
		AUTOMATIC_CLEANUP = 1 << 0
	};

	Type m_type;
	Bitset<U8> m_flags;
};

} // end namespace anki

#endif
