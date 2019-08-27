// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/StaticGeometryNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ModelResource.h>

namespace anki
{

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
	for(U32 i = 1; i < m_modelPatch->getSubMeshCount(); i++)
	{
		SpatialComponent* spatial = newComponent<SpatialComponent>(this, &m_modelPatch->getBoundingShapeSub(i));

		spatial->setSpatialOrigin(m_modelPatch->getBoundingShapeSub(i).getCenter());
	}

	// Create render component
	MaterialRenderComponent* rcomp = newComponent<MaterialRenderComponent>(this, m_modelPatch->getMaterial());
	rcomp->setup([](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) { ANKI_ASSERT(!"TODO"); }, this, 0);

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
	for(const ModelPatch& patch : m_model->getModelPatches())
	{
		StaticGeometryPatchNode* node;
		ANKI_CHECK(getSceneGraph().newSceneNode<StaticGeometryPatchNode>(CString(), node, &patch));

		++i;
	}

	return Error::NONE;
}

} // end namespace anki
