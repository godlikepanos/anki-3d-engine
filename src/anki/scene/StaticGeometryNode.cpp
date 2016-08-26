// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/StaticGeometryNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

//==============================================================================
// StaticGeometryRenderComponent                                               =
//==============================================================================

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

	Error buildRendering(RenderingBuildInfo& data) const override
	{
		return m_node->buildRendering(data);
	}
};

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(
	SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{
}

//==============================================================================
Error StaticGeometryPatchNode::init(const ModelPatch* modelPatch)
{
	ANKI_ASSERT(modelPatch);

	m_modelPatch = modelPatch;

	// Create spatial components
	for(U i = 1; i < m_modelPatch->getSubMeshesCount(); i++)
	{
		SpatialComponent* spatial =
			getSceneAllocator().newInstance<SpatialComponent>(
				this, &m_modelPatch->getBoundingShapeSub(i));

		addComponent(spatial, true);

		spatial->setSpatialOrigin(
			m_modelPatch->getBoundingShapeSub(i).getCenter());
		spatial->setAutomaticCleanup(true);
	}

	// Create render component
	RenderComponent* rcomp =
		getSceneAllocator().newInstance<StaticGeometryRenderComponent>(this);
	addComponent(rcomp, true);

	return ErrorCode::NONE;
}

//==============================================================================
Error StaticGeometryPatchNode::buildRendering(RenderingBuildInfo& data) const
{
	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawCount;
	ResourceGroupPtr grResources;
	PipelinePtr ppline;

	m_modelPatch->getRenderingDataSub(data.m_key,
		WeakArray<U8>(const_cast<U8*>(data.m_subMeshIndicesArray),
										  data.m_subMeshIndicesCount),
		grResources,
		ppline,
		indicesCountArray,
		indicesOffsetArray,
		drawCount);

	data.m_cmdb->bindPipeline(ppline);

	if(drawCount == 1)
	{
		data.m_cmdb->bindResourceGroup(
			grResources, 0, data.m_dynamicBufferInfo);

		data.m_cmdb->drawElements(
			indicesCountArray[0], 1, indicesOffsetArray[0] / sizeof(U16));
	}
	else if(drawCount == 0)
	{
		ANKI_ASSERT(0);
	}
	else
	{
		// TODO Make it indirect
		ANKI_ASSERT(0 && "TODO");
	}

	return ErrorCode::NONE;
}

//==============================================================================
// StaticGeometryNode                                                          =
//==============================================================================

//==============================================================================
StaticGeometryNode::StaticGeometryNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

//==============================================================================
StaticGeometryNode::~StaticGeometryNode()
{
}

//==============================================================================
Error StaticGeometryNode::init(const CString& filename)
{
	ANKI_CHECK(getResourceManager().loadResource(filename, m_model));

	U i = 0;
	for(const ModelPatch* patch : m_model->getModelPatches())
	{
		StaticGeometryPatchNode* node;
		ANKI_CHECK(getSceneGraph().newSceneNode<StaticGeometryPatchNode>(
			CString(), node, patch));

		++i;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
