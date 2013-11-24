#ifndef ANKI_SCENE_SCENE_COMPONENT_H
#define ANKI_SCENE_SCENE_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/core/Timestamp.h"

namespace anki {

/// Scene node component
class SceneComponent
{
public:
	// The type of the components
	enum Type
	{
		COMPONENT_NONE,
		FRUSTUM_COMPONENT,
		MOVE_COMPONENT,
		RENDER_COMPONENT,
		SPATIAL_COMPONENT,
		LIGHT_COMPONENT,
		INSTANCE_COMPONENT,
		RIGID_BODY,
		LAST_COMPONENT_ID = RIGID_BODY
	};

	/// The update type
	enum UpdateType
	{
		/// The update happens in the thread safe sync section
		SYNC_UPDATE, 
		
		/// The update happens in a thread. This should not touch data that 
		/// belong to other nodes
		ASYNC_UPDATE 
	};

	/// Construct the scene component. The x is bogus
	SceneComponent(Type type_, SceneNode* node)
		: type(type_)
	{}

	Type getType() const
	{
		return (Type)type;
	}

	Timestamp getTimestamp() const
	{
		return timestamp;
	}

	/// Do some reseting before frame starts
	virtual void reset()
	{}

	/// Do some updating
	/// @return true if an update happened
	virtual Bool update(SceneNode& node, F32 prevTime, F32 crntTime, 
		UpdateType updateType)
	{
		return false;
	}

	/// Called only by the SceneGraph
	Bool updateReal(SceneNode& node, F32 prevTime, F32 crntTime,
		UpdateType updateType);

	template<typename TComponent>
	TComponent& downCast()
	{
		ANKI_ASSERT(TComponent::getGlobType() == getType());
		TComponent* out = staticCast<TComponent*>(this);
		return *out;
	}

protected:
	Timestamp timestamp; ///< Indicates when an update happened

private:
	U8 type;
};

} // end namespace anki

#endif
