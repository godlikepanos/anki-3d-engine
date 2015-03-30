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
	:	RenderComponent(node),
		m_node(node)
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
:	SceneNode(scene)
{}

//==============================================================================
Error StaticGeometryPatchNode::create(
	const CString& name, const ModelPatchBase* modelPatch)
{
	ANKI_ASSERT(modelPatch);
	Error err = ErrorCode::NONE;

	m_modelPatch = modelPatch;

	err = SceneNode::create(name);
	if(err) return err;

	// Create spatial components
	for(U i = 1; i < m_modelPatch->getSubMeshesCount() && !err; i++)
	{
		SpatialComponent* spatial =
			getSceneAllocator().newInstance<SpatialComponent>(
			this, &m_modelPatch->getBoundingShapeSub(i));

		if(spatial == nullptr) return ErrorCode::OUT_OF_MEMORY;


		err = addComponent(spatial);
		if(err) return err;

		spatial->setSpatialOrigin(
			m_modelPatch->getBoundingShapeSub(i).getCenter());
		spatial->setAutomaticCleanup(true);
	}

	// Create render component
	RenderComponent* rcomp = 
		getSceneAllocator().newInstance<StaticGeometryRenderComponent>(this);
	if(rcomp == nullptr) return ErrorCode::OUT_OF_MEMORY;
	
	err = addComponent(rcomp);
	if(err) return err;

	return err;
}

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{}

//==============================================================================
Error StaticGeometryPatchNode::buildRendering(RenderingBuildData& data)
{
	Error err = ErrorCode::NONE;

	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawCount;
	CommandBufferHandle vertJobs;
	PipelineHandle ppline;

	err = m_modelPatch->getRenderingDataSub(
		data.m_key, vertJobs, ppline, 
		data.m_subMeshIndicesArray, data.m_subMeshIndicesCount, 
		indicesCountArray, indicesOffsetArray, drawCount);
	if(err)
	{
		return err;
	}

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

	return err;
}

//==============================================================================
// StaticGeometryNode                                                          =
//==============================================================================

//==============================================================================
StaticGeometryNode::StaticGeometryNode(SceneGraph* scene)
:	SceneNode(scene)
{}

//==============================================================================
Error StaticGeometryNode::create(const CString& name, const CString& filename)
{
	Error err = SceneNode::create(name);

	if(!err)
	{
		err = m_model.load(filename, &getResourceManager());
	}

	if(!err)
	{
		U i = 0;
		for(const ModelPatchBase* patch : m_model->getModelPatches())
		{
			StringAuto newname(getSceneAllocator());
			
			err = newname.sprintf("%s_%u", &name[0], i);
			if(err)
			{
				break;
			}

			StaticGeometryPatchNode* node;
			err = getSceneGraph().newSceneNode<StaticGeometryPatchNode>(
				newname.toCString(), node, patch);

			if(err)
			{
				break;
			}

			++i;
		}
	}

	return err;
}

//==============================================================================
StaticGeometryNode::~StaticGeometryNode()
{}

} // end namespace anki
