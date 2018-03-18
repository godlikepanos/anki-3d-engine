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
class StaticGeometryRenderComponent : public RenderComponent
{
public:
	StaticGeometryPatchNode* m_node;

	StaticGeometryRenderComponent(StaticGeometryPatchNode* node)
		: RenderComponent(node, node->m_modelPatch->getMaterial())
		, m_node(node)
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
	newComponent<StaticGeometryRenderComponent>(this);

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
