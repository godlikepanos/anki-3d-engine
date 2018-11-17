// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/StaticGeometryNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ModelResource.h>

namespace anki
{

/// The implementation of static geometry node renderable component.
class StaticGeometryPatchNode::RenderComponent : public MaterialRenderComponent
{
public:
	RenderComponent(SceneNode* node)
		: MaterialRenderComponent(node, static_cast<StaticGeometryPatchNode*>(node)->m_modelPatch->getMaterial())
	{
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const override
	{
		ANKI_ASSERT(!"TODO");
	}
};

StaticGeometryPatchNode::StaticGeometryPatchNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

StaticGeometryPatchNode::~StaticGeometryPatchNode()
{
}

Error StaticGeometryPatchNode::init(const ModelPatch* modelPatch)
{
	ANKI_ASSERT(modelPatch);

	m_modelPatch = modelPatch;

	// Create spatial components
	for(U i = 1; i < m_modelPatch->getSubMeshCount(); i++)
	{
		SpatialComponent* spatial = newComponent<SpatialComponent>(this, &m_modelPatch->getBoundingShapeSub(i));

		spatial->setSpatialOrigin(m_modelPatch->getBoundingShapeSub(i).getCenter());
	}

	// Create render component
	newComponent<RenderComponent>(this);

	return Error::NONE;
}

StaticGeometryNode::StaticGeometryNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

StaticGeometryNode::~StaticGeometryNode()
{
}

Error StaticGeometryNode::init(const CString& filename)
{
	ANKI_CHECK(getResourceManager().loadResource(filename, m_model));

	U i = 0;
	for(const ModelPatch* patch : m_model->getModelPatches())
	{
		StaticGeometryPatchNode* node;
		ANKI_CHECK(getSceneGraph().newSceneNode<StaticGeometryPatchNode>(CString(), node, patch));

		++i;
	}

	return Error::NONE;
}

} // end namespace anki
