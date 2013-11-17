#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/gl/Drawcall.h"

namespace anki {

//==============================================================================
// StaticGeometrySpatial                                                       =
//==============================================================================

//==============================================================================
StaticGeometrySpatial::StaticGeometrySpatial(
	SceneNode* node, const Obb* obb_)
	: SpatialComponent(node), obb(obb_)
{}

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(
	const char* name, SceneGraph* scene, const ModelPatchBase* modelPatch_)
	:	SceneNode(name, scene),
		SpatialComponent(this),
		RenderComponent(this),
		modelPatch(modelPatch_)
{
	addComponent(static_cast<SpatialComponent*>(this));
	addComponent(static_cast<RenderComponent*>(this));

	ANKI_ASSERT(modelPatch);
	RenderComponent::init();

	// Check if multimesh
	if(modelPatch->getSubMeshesCount() > 1)
	{
		// If multimesh create additional spatial components

		obb = &modelPatch->getBoundingShapeSub(0);

		for(U i = 1; i < modelPatch->getSubMeshesCount(); i++)
		{
			StaticGeometrySpatial* spatial =
				getSceneAllocator().newInstance<StaticGeometrySpatial>(
				this, &modelPatch->getBoundingShapeSub(i));

			addComponent(static_cast<SpatialComponent*>(spatial));
		}
	}
	else
	{
		// If not multimesh then set the current spatial component

		obb = &modelPatch->getBoundingShape();
	}
}

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{
	U i = 0;
	iterateComponentsOfType<SpatialComponent>([&](SpatialComponent& spatial)
	{
		if(i != 0)
		{
			getSceneAllocator().deleteInstance(&spatial);
		}
		++i;
	});
}

//==============================================================================
void StaticGeometryPatchNode::getRenderingData(
	const PassLodKey& key, 
	const U32* subMeshIndicesArray, U subMeshIndicesCount,
	const Vao*& vao, const ShaderProgram*& prog,
	Drawcall& dc)
{
	dc.primitiveType = GL_TRIANGLES;
	dc.indicesType = GL_UNSIGNED_SHORT;

	U spatialsCount = 0;
	iterateComponentsOfType<SpatialComponent>([&](SpatialComponent&)
	{
		++spatialsCount;
	});

	dc.instancesCount = spatialsCount;

	modelPatch->getRenderingDataSub(key, vao, prog, 
		subMeshIndicesArray, subMeshIndicesCount, 
		dc.countArray, dc.offsetArray, dc.drawCount);
}

//==============================================================================
// StaticGeometryNode                                                          =
//==============================================================================

//==============================================================================
StaticGeometryNode::StaticGeometryNode(
	const char* name, SceneGraph* scene, const char* filename)
	: SceneNode(name, scene)
{
	model.load(filename);

	U i = 0;
	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		std::string name_ = name + std::to_string(i);

		StaticGeometryPatchNode* node;
		getSceneGraph().newSceneNode(node, name_.c_str(), patch);

		++i;
	}
}

//==============================================================================
StaticGeometryNode::~StaticGeometryNode()
{}

} // end namespace anki
