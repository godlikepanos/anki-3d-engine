#ifndef ANKI_SCENE_SCENE_COMPONENT_H
#define ANKI_SCENE_SCENE_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/core/Timestamp.h"
#include "anki/util/Visitor.h"

namespace anki {

// Forward of all components
class FrustumComponent;
class MoveComponent;
class RenderComponent;
class SpatialComponent;
class LightComponent;
class RigidBody;

class SceneComponent;

typedef VisitableCommonBase<
	SceneComponent, // Base
	FrustumComponent,
	MoveComponent,
	RenderComponent,
	SpatialComponent,
	LightComponent,
	RigidBody> SceneComponentVisitable;

/// Scene node component
class SceneComponent: public SceneComponentVisitable
{
public:
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
	template<typename T>
	SceneComponent(const T* x, SceneNode* node)
	{
		setupVisitable<T>(x);
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

	Timestamp getTimestamp() const
	{
		return timestamp;
	}

	I getTypeId() const
	{
		return getVisitableTypeId();
	}

	/// Get the type ID of a derived class
	template<typename ComponentDerived>
	static I getTypeIdOf()
	{
		return getVariadicTypeId<ComponentDerived>();
	}

protected:
	Timestamp timestamp;
};

} // end namespace anki

#endif
