#ifndef ANKI_SCENE_STATIC_GEOMETRY_NODE_H
#define ANKI_SCENE_STATIC_GEOMETRY_NODE_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Renderable.h"

namespace anki {

/// Part of the static geometry
class StaticGeometryPatchNode: public SceneNode, public Spatial
{
public:
	/// @name Constructors/Destructor
	/// @{
	StaticGeometryPatchNode(const Obb& obb,
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

public:
	Obb obb;
};

/// XXX
class StaticGeometryNode: public SceneNode, public Spatial, public Renderable
{
public:

private:
	SceneVector<StaticGeometryPatchNode*> patches;
};

} // end namespace anki

#endif
