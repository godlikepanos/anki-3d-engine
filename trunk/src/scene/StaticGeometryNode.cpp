#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
// StaticGeometrySpatial                                                       =
//==============================================================================

//==============================================================================
StaticGeometrySpatial::StaticGeometrySpatial(const Obb* obb,
	SceneNode& node)
	: SpatialComponent(&node, obb)
{}

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(
	const char* name, SceneGraph* scene, const ModelPatchBase* modelPatch_)
	:	SceneNode(name, scene),
		SpatialComponent(this, &modelPatch_->getBoundingShape()),
		RenderComponent(this),
		modelPatch(modelPatch_)
{
	sceneNodeProtected.spatialC = this;
	sceneNodeProtected.renderC = this;

	ANKI_ASSERT(modelPatch);
	RenderComponent::init();

	// For all submeshes create a StaticGeometrySp[atial
	if(modelPatch->getSubMeshesCount() > 1)
	{
		for(U i = 0; i < modelPatch->getSubMeshesCount(); i++)
		{
			StaticGeometrySpatial* spatial =
				getSceneAllocator().newInstance<StaticGeometrySpatial>(
				&modelPatch->getBoundingShapeSub(i), *this);

			SpatialComponent::addChild(spatial);
		}
	}
}

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{
	visitSubSpatials([&](SpatialComponent& spatial)
	{
		getSceneAllocator().deleteInstance(&spatial);
	});
}

//==============================================================================
// StaticGeometryNode                                                          =
//==============================================================================

//==============================================================================
StaticGeometryNode::StaticGeometryNode(
	const char* name, SceneGraph* scene, const char* filename)
	: SceneNode(name, scene), patches(getSceneAllocator())
{
	model.load(filename);

	patches.reserve(model->getModelPatches().size());

	U i = 0;
	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		std::string name_ = name + std::to_string(i);

		StaticGeometryPatchNode* node;
		getSceneGraph().newSceneNode(node, name_.c_str(), patch);

		patches.push_back(node);
		++i;
	}
}

//==============================================================================
StaticGeometryNode::~StaticGeometryNode()
{
	for(StaticGeometryPatchNode* patch : patches)
	{
		getSceneGraph().deleteSceneNode(patch);
	}
}

} // end namespace anki
