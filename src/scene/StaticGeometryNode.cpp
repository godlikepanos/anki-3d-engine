#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/Scene.h"

namespace anki {

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(const Obb& obb_,
	const char* name, Scene* scene)
	: SceneNode(name, scene), Spatial(&obb), obb(obb_)
{}

} // end namespace anki
