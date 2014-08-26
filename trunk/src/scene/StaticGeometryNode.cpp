// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/StaticGeometryNode.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
// StaticGeometrySpatial                                                       =
//==============================================================================

//==============================================================================
StaticGeometrySpatial::StaticGeometrySpatial(
	SceneNode* node, const Obb* obb)
	: SpatialComponent(node), m_obb(obb)
{}

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(
	const char* name, SceneGraph* scene, const ModelPatchBase* modelPatch)
	:	SceneNode(name, scene),
		SpatialComponent(this),
		RenderComponent(this),
		m_modelPatch(modelPatch)
{
	addComponent(static_cast<SpatialComponent*>(this));
	addComponent(static_cast<RenderComponent*>(this));

	ANKI_ASSERT(m_modelPatch);
	RenderComponent::init();

	// Check if multimesh
	if(m_modelPatch->getSubMeshesCount() > 1)
	{
		// If multimesh create additional spatial components

		m_obb = &m_modelPatch->getBoundingShapeSub(0);

		for(U i = 1; i < m_modelPatch->getSubMeshesCount(); i++)
		{
			StaticGeometrySpatial* spatial =
				getSceneAllocator().newInstance<StaticGeometrySpatial>(
				this, &m_modelPatch->getBoundingShapeSub(i));

			addComponent(static_cast<SpatialComponent*>(spatial));
		}
	}
	else
	{
		// If not multimesh then set the current spatial component

		m_obb = &modelPatch->getBoundingShape();
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
void StaticGeometryPatchNode::buildRendering(RenderingBuildData& data)
{
	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawCount;
	GlCommandBufferHandle vertJobs;
	GlProgramPipelineHandle ppline;

	m_modelPatch->getRenderingDataSub(
		data.m_key, vertJobs, ppline, 
		data.m_subMeshIndicesArray, data.m_subMeshIndicesCount, 
		indicesCountArray, indicesOffsetArray, drawCount);

	ppline.bind(data.m_jobs);
	data.m_jobs.pushBackOtherCommandBuffer(vertJobs);

	if(drawCount == 1)
	{
		data.m_jobs.drawElements(
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
}

//==============================================================================
// StaticGeometryNode                                                          =
//==============================================================================

//==============================================================================
StaticGeometryNode::StaticGeometryNode(
	const char* name, SceneGraph* scene, const char* filename)
	: SceneNode(name, scene)
{
	m_model.load(filename);

	U i = 0;
	for(const ModelPatchBase* patch : m_model->getModelPatches())
	{
		std::string newname = name + std::to_string(i);

		StaticGeometryPatchNode* node = 
			getSceneGraph().newSceneNode<StaticGeometryPatchNode>(
			newname.c_str(), patch);
		(void)node;

		++i;
	}
}

//==============================================================================
StaticGeometryNode::~StaticGeometryNode()
{}

} // end namespace anki
