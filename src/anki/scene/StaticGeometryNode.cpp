// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/StaticGeometryNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/Model.h>

namespace anki
{

/// The implementation of static geometry node renderable component.
class StaticGeometryRenderComponent : public RenderComponent
{
public:
	StaticGeometryPatchNode* m_node;

	StaticGeometryRenderComponent(StaticGeometryPatchNode* node)
		: RenderComponent(node, &node->m_modelPatch->getMaterial())
		, m_node(node)
	{
	}

	Error buildRendering(const RenderingBuildInfoIn& in, RenderingBuildInfoOut& out) const override
	{
		return m_node->buildRendering(in, out);
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
	for(U i = 1; i < m_modelPatch->getSubMeshesCount(); i++)
	{
		SpatialComponent* spatial =
			getSceneAllocator().newInstance<SpatialComponent>(this, &m_modelPatch->getBoundingShapeSub(i));

		addComponent(spatial, true);

		spatial->setSpatialOrigin(m_modelPatch->getBoundingShapeSub(i).getCenter());
		spatial->setAutomaticCleanup(true);
	}

	// Create render component
	RenderComponent* rcomp = getSceneAllocator().newInstance<StaticGeometryRenderComponent>(this);
	addComponent(rcomp, true);

	return ErrorCode::NONE;
}

Error StaticGeometryPatchNode::buildRendering(const RenderingBuildInfoIn& in, RenderingBuildInfoOut& out) const
{
	ANKI_ASSERT(!"TODO");
	return ErrorCode::NONE;
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

	return ErrorCode::NONE;
}

} // end namespace anki
