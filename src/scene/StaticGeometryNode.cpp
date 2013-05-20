#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
// StaticGeometrySpatial                                                       =
//==============================================================================

//==============================================================================
StaticGeometrySpatial::StaticGeometrySpatial(const Obb* obb,
	const SceneAllocator<U8>& alloc)
	: Spatial(obb, alloc)
{}

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(
	const char* name, SceneGraph* scene, const ModelPatchBase* modelPatch_)
	:	SceneNode(name, scene, nullptr),
		Spatial(&modelPatch_->getBoundingShape(), getSceneAllocator()),
		Renderable(getSceneAllocator()),
		modelPatch(modelPatch_)
{
	sceneNodeProtected.spatial = this;
	sceneNodeProtected.renderable = this;

	ANKI_ASSERT(modelPatch);
	Renderable::init(*this);

	// For all submeshes create a StaticGeometrySp[atial
	if(modelPatch->getSubMeshesCount() > 1)
	{
		spatialProtected.subSpatials.resize(modelPatch->getSubMeshesCount());

		for(U i = 0; i < modelPatch->getSubMeshesCount(); i++)
		{
			StaticGeometrySpatial* spatial =
				ANKI_NEW(StaticGeometrySpatial, getSceneAllocator(),
				&modelPatch->getBoundingShapeSub(i), getSceneAllocator());

			spatialProtected.subSpatials[i] = spatial;
		}
	}
}

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{
	for(Spatial* spatial : spatialProtected.subSpatials)
	{
		ANKI_DELETE(spatial, getSceneAllocator());
	}
}

//==============================================================================
// StaticGeometryNode                                                          =
//==============================================================================

//==============================================================================
StaticGeometryNode::StaticGeometryNode(
	const char* name, SceneGraph* scene, const char* filename)
	: SceneNode(name, scene, nullptr), patches(getSceneAllocator())
{
	model.load(filename);

	patches.reserve(model->getModelPatches().size());

	U i = 0;
	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		std::string name_ = name + std::to_string(i);

		StaticGeometryPatchNode* node = ANKI_NEW(
			StaticGeometryPatchNode, getSceneAllocator(),
			name_.c_str(), scene, patch);

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
