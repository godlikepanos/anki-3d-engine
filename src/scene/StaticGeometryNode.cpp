// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
// StaticGeometryRenderComponent                                               =
//==============================================================================

/// The implementation of static geometry node renderable component.
class StaticGeometryRenderComponent: public RenderComponent
{
public:
	StaticGeometryPatchNode* m_node;

	StaticGeometryRenderComponent(StaticGeometryPatchNode* node)
		: RenderComponent(node)
		, m_node(node)
	{}

	Error buildRendering(RenderingBuildData& data) override
	{
		return m_node->buildRendering(data);
	}

	const Material& getMaterial()
	{
		return m_node->m_modelPatch->getMaterial();
	}
};

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(SceneGraph* scene)
	: SceneNode(scene)
{}

//==============================================================================
Error StaticGeometryPatchNode::create(
	const CString& name, const ModelPatch* modelPatch)
{
	ANKI_ASSERT(modelPatch);

	m_modelPatch = modelPatch;
	ANKI_CHECK(SceneNode::create(name));

	// Create spatial components
	for(U i = 1; i < m_modelPatch->getSubMeshesCount(); i++)
	{
		SpatialComponent* spatial =
			getSceneAllocator().newInstance<SpatialComponent>(
			this, &m_modelPatch->getBoundingShapeSub(i));

		addComponent(spatial);

		spatial->setSpatialOrigin(
			m_modelPatch->getBoundingShapeSub(i).getCenter());
		spatial->setAutomaticCleanup(true);
	}

	// Create render component
	RenderComponent* rcomp =
		getSceneAllocator().newInstance<StaticGeometryRenderComponent>(this);
	addComponent(rcomp);

	return ErrorCode::NONE;
}

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{}

//==============================================================================
Error StaticGeometryPatchNode::buildRendering(RenderingBuildData& data)
{
	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawCount;
	BufferPtr vertBuff, indexBuff;
	PipelinePtr ppline;

	m_modelPatch->getRenderingDataSub(
		data.m_key,
		SArray<U8>(const_cast<U8*>(data.m_subMeshIndicesArray),
			data.m_subMeshIndicesCount),
		vertBuff,
		indexBuff,
		ppline,
		indicesCountArray,
		indicesOffsetArray,
		drawCount);

	ppline.bind(data.m_cmdb);

	if(drawCount == 1)
	{
		data.m_cmdb.bindVertexBuffer(0, vertBuff, 0);
		data.m_cmdb.bindIndexBuffer(indexBuff, 0);

		data.m_cmdb.drawElements(
			data.m_key.m_tessellation ? GL_PATCHES : GL_TRIANGLES,
			sizeof(U16),
			indicesCountArray[0],
			1,
			indicesOffsetArray[0] / sizeof(U16));
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
StaticGeometryNode::StaticGeometryNode(SceneGraph* scene)
	: SceneNode(scene)
{}

//==============================================================================
StaticGeometryNode::~StaticGeometryNode()
{}

//==============================================================================
Error StaticGeometryNode::create(const CString& name, const CString& filename)
{
	ANKI_CHECK(SceneNode::create(name));

	ANKI_CHECK(m_model.load(filename, &getResourceManager()));

	U i = 0;
	for(const ModelPatch* patch : m_model->getModelPatches())
	{
		StringAuto newname(getFrameAllocator());

		newname.sprintf("%s_%u", &name[0], i);

		StaticGeometryPatchNode* node;
		ANKI_CHECK(getSceneGraph().newSceneNode<StaticGeometryPatchNode>(
			newname.toCString(), node, patch));

		++i;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
