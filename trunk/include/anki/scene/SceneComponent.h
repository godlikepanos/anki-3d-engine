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
	/// Construct the scene component. The x is bogus
	template<typename T>
	SceneComponent(const T* x)
	{
		setupVisitable<T>(x);
	}

	/// Do some reseting before frame starts
	virtual void reset()
	{}

	/// Do some updating on the thread safe sync section
	/// @return true if an update happened
	virtual Bool syncUpdate(SceneNode& node, F32 prevTime, F32 crntTime)
	{
		return false;
	}

	/// Do some updating
	/// @return true if an update happened
	virtual Bool update(SceneNode& node, F32 prevTime, F32 crntTime)
	{
		return false;
	}

	/// Called only by the SceneGraph
	void updateReal(SceneNode& node, F32 prevTime, F32 crntTime)
	{
		if(update(node, prevTime, crntTime))
		{
			timestamp = getGlobTimestamp();
		}
	}

	Timestamp getTimestamp() const
	{
		return timestamp;
	}

protected:
	Timestamp timestamp;
};

} // end namespace anki

#endif
