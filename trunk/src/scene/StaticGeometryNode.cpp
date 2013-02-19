#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/Scene.h"

namespace anki {

//==============================================================================
// StaticGeometrySpatialNode                                                   =
//==============================================================================

//==============================================================================
StaticGeometrySpatialNode::StaticGeometrySpatialNode(const Obb& obb_,
	const char* name, Scene* scene)
	: SceneNode(name, scene), Spatial(&obb), obb(obb_)
{
	sceneNodeProtected.spatial = this;
}

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(
	const ModelPatchBase* modelPatch_, const char* name, Scene* scene)
	:	SceneNode(name, scene),
		Spatial(&obb),
		Renderable(getSceneAllocator()),
		modelPatch(modelPatch_)
{
	sceneNodeProtected.spatial = this;
	sceneNodeProtected.renderable = this;

	ANKI_ASSERT(modelPatch);
	Renderable::init(*this);

	obb = modelPatch->getBoundingShape();

	if(modelPatch->getSubMeshesCount() > 1)
	{
		spatials.reserve(modelPatch->getSubMeshesCount());

		for(U i = 0; i < modelPatch->getSubMeshesCount(); i++)
		{
			std::string aName = name + std::to_string(i);

			StaticGeometrySpatialNode* node =
				ANKI_NEW(StaticGeometrySpatialNode, getSceneAllocator(),
				modelPatch->getBoundingShapeSub(i), aName.c_str(), scene);

			spatials.push_back(node);
		}
	}
}

} // end namespace anki
