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
:	SpatialComponent(node), 
	m_obb(obb)
{}

//==============================================================================
// StaticGeometryPatchNode                                                     =
//==============================================================================

//==============================================================================
StaticGeometryPatchNode::StaticGeometryPatchNode(SceneGraph* scene)
:	SceneNode(scene),
	SpatialComponent(this),
	RenderComponent(this)
{}

//==============================================================================
Error StaticGeometryPatchNode::create(
	const CString& name, const ModelPatchBase* modelPatch)
{
	ANKI_ASSERT(modelPatch);
	Error err = ErrorCode::NONE;

	m_modelPatch = modelPatch;

	err = SceneNode::create(name);
	
	if(!err)
	{
		err = addComponent(static_cast<SpatialComponent*>(this));
	}

	if(!err)
	{
		err = addComponent(static_cast<RenderComponent*>(this));
	}

	if(!err)
	{	
		err = RenderComponent::create();
	}

	if(!err)
	{
		// Check if multimesh
		if(m_modelPatch->getSubMeshesCount() > 1)
		{
			// If multimesh create additional spatial components

			m_obb = &m_modelPatch->getBoundingShapeSub(0);

			for(U i = 1; i < m_modelPatch->getSubMeshesCount() && !err; i++)
			{
				StaticGeometrySpatial* spatial =
					getSceneAllocator().newInstance<StaticGeometrySpatial>(
					this, &m_modelPatch->getBoundingShapeSub(i));

				if(spatial)
				{
					err = addComponent(static_cast<SpatialComponent*>(spatial));
				}
				else
				{
					err = ErrorCode::OUT_OF_MEMORY;
				}
			}
		}
		else
		{
			// If not multimesh then set the current spatial component

			m_obb = &modelPatch->getBoundingShape();
		}
	}

	return err;
}

//==============================================================================
StaticGeometryPatchNode::~StaticGeometryPatchNode()
{
	U i = 0;
	Error err = iterateComponentsOfType<SpatialComponent>([&](
		SpatialComponent& spatial)
	{
		if(i != 0)
		{
			getSceneAllocator().deleteInstance(&spatial);
		}
		++i;

		return ErrorCode::NONE;
	});

	(void)err;
}

//==============================================================================
Error StaticGeometryPatchNode::buildRendering(RenderingBuildData& data)
{
	Error err = ErrorCode::NONE;

	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawCount;
	GlCommandBufferHandle vertJobs;
	GlPipelineHandle ppline;

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
{
}

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
			SceneString newname;
			
			err = newname.sprintf(getSceneAllocator(), "%s_%u", &name[0], i);
			if(err)
			{
				break;
			}

			StaticGeometryPatchNode* node;
			err = getSceneGraph().newSceneNode<StaticGeometryPatchNode>(
				newname.toCString(), node, patch);
			
			newname.destroy(getSceneAllocator());

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
