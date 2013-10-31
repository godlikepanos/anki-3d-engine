#ifndef ANKI_SCENE_SCENE_COMPONENT_H
#define ANKI_SCENE_SCENE_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/core/Timestamp.h"

namespace anki {

/// Scene node component
class SceneComponent
{
public:
	/// Do some updating on the thread safe sync section
	virtual void syncUpdate(SceneNode& node, F32 prevTime, F32 crntTime)
	{}

	/// Do some updating
	virtual void update(SceneNode& node, F32 prevTime, F32 crntTime)
	{}

	/// Called only by the SceneGraph
	void updateReal(SceneNode& node, F32 prevTime, F32 crntTime)
	{
		update(node, prevTime, crntTime);
		timestamp = getTimestamp();
	}

	Timestamp getTimestamp() const
	{
		return timestamp;
	}

private:
	Timestamp timestamp;
};

} // end namespace anki

#endif
