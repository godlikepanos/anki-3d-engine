#ifndef ANKI_SCENE_STATIC_GEOMETRY_NODE_H
#define ANKI_SCENE_STATIC_GEOMETRY_NODE_H

#include "anki/scene/Common.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Renderable.h"

namespace anki {

/// XXX
class StaticGeometryPatchNode: public SceneNode, public Spatial
{
public:
	/// @name Constructors/Destructor
	/// @{
	StaticGeometryPatchNode(
		const char* name, Scene* scene); // Scene
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getSpatial()
	Spatial* getSpatial()
	{
		return this;
	}
	/// @}
};

/// XXX
class StaticGeometryNode: public SceneNode, public Spatial, public Renderable
{
public:

private:
	SceneVector<StaticGeometryPatch*> patches;
};

} // end namespace anki

#endif
