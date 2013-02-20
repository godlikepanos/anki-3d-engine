#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
// StaticGeometrySpatialNode                                                   =
//==============================================================================

//==============================================================================
StaticGeometrySpatialNode::StaticGeometrySpatialNode(const Obb& obb,
	const char* name, SceneGraph* scene)
	: SceneNode(name, scene), Spatial(&obb)
{
	sceneNodeProtected.spatial = this;
}

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(
	const ModelPatchBase* modelPatch_, const char* name, SceneGraph* scene)
	:	SceneNode(name, scene),
		Spatial(&modelPatch->getBoundingShape()),
		Renderable(getSceneAllocator()),
		modelPatch(modelPatch_)
{
	sceneNodeProtected.spatial = this;
	sceneNodeProtected.renderable = this;

	ANKI_ASSERT(modelPatch);
	Renderable::init(*this);

	// For all submeshes create a StaticGeometrySp[atialNode
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

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{
	for(StaticGeometrySpatialNode* node : spatials)
	{
		ANKI_DELETE(node, getSceneAllocator());
	}
}

//==============================================================================
// StaticGeometryNode                                                          =
//==============================================================================

//==============================================================================
StaticGeometryNode::StaticGeometryNode(const char* filename,
	const char* name, SceneGraph* scene)
	: SceneNode(name, scene)
{
	model.load(filename);

	patches.reserve(model->getModelPatches().size());

	U i = 0;
	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		std::string name_ = name + std::to_string(i);

		StaticGeometryPatchNode* node = 
			ANKI_NEW(StaticGeometryPatchNode, getSceneAllocator(),
			patch, name_.c_str(), scene);

		patches.push_back(node);
		++i;
	}
}

//==============================================================================
StaticGeometryNode::~StaticGeometryNode()
{
	for(StaticGeometryPatchNode* patch : patches)
	{
		ANKI_DELETE(patch, getSceneAllocator());
	}
}

} // end namespace anki
