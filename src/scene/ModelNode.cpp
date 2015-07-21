// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/ModelNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/InstanceNode.h"
#include "anki/scene/BodyComponent.h"
#include "anki/scene/Misc.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
// ModelPatchRenderComponent                                                   =
//==============================================================================

/// Render component implementation.
class ModelPatchRenderComponent: public RenderComponent
{
public:
	ModelPatchNode* m_node;

	ModelPatchRenderComponent(ModelPatchNode* node)
		: RenderComponent(node)
		, m_node(node)
	{}

	ANKI_USE_RESULT Error buildRendering(RenderingBuildData& data) override
	{
		return m_node->buildRendering(data);
	}

	const Material& getMaterial() override
	{
		return m_node->m_modelPatch->getMaterial();
	}

	void getRenderWorldTransform(U index, Transform& trf) override
	{
		m_node->getRenderWorldTransform(index, trf);
	}

	Bool getHasWorldTransforms() override
	{
		return true;
	}
};

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(SceneGraph* scene)
	: SceneNode(scene)
{}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{
	for(SpatialComponent* sp : m_spatials)
	{
		getSceneAllocator().deleteInstance(sp);
	}

	m_spatials.destroy(getSceneAllocator());
}

//==============================================================================
Error ModelPatchNode::create(const CString& name,
	const ModelPatch* modelPatch)
{
	ANKI_ASSERT(modelPatch);
	ANKI_CHECK(SceneNode::create(name));

	m_modelPatch = modelPatch;

	// Spatial component
	SceneComponent* comp = getSceneAllocator().newInstance<SpatialComponent>(
		this, &m_obb);

	addComponent(comp, true);

	// Render component
	RenderComponent* rcomp =
		getSceneAllocator().newInstance<ModelPatchRenderComponent>(this);
	comp = rcomp;

	addComponent(comp, true);
	ANKI_CHECK(rcomp->create());

	return ErrorCode::NONE;
}

//==============================================================================
Error ModelPatchNode::buildRendering(RenderingBuildData& data)
{
	// That will not work on multi-draw and instanced at the same time. Make
	// sure that there is no multi-draw anywhere
	ANKI_ASSERT(m_modelPatch->getSubMeshesCount() == 0);

	auto instancesCount = data.m_subMeshIndicesCount;

	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawcallCount;

	PipelinePtr ppline;
	ResourceGroupPtr grResources;

	m_modelPatch->getRenderingDataSub(
		data.m_key,
		SArray<U8>(),
		grResources,
		ppline,
		indicesCountArray,
		indicesOffsetArray,
		drawcallCount);

	// Cannot accept multi-draw
	ANKI_ASSERT(drawcallCount == 1);

	// Set jobs
	data.m_cmdb->bindPipeline(ppline);
	data.m_cmdb->bindResourceGroup(grResources);

	// Drawcall
	U32 offset = indicesOffsetArray[0] / sizeof(U16);
	data.m_cmdb->drawElements(
		indicesCountArray[0],
		instancesCount,
		offset);

	return ErrorCode::NONE;
}

//==============================================================================
void ModelPatchNode::getRenderWorldTransform(U index, Transform& trf)
{
	SceneNode* parent = getParent();
	ANKI_ASSERT(parent);
	MoveComponent& move = parent->getComponent<MoveComponent>();

	if(index == 0)
	{
		// Asking for the first instance which is this
		trf = move.getWorldTransform();
	}
	else
	{
		// Asking for a next instance
		SceneNode* parent = getParent();
		ANKI_ASSERT(parent);
		ModelNode* mnode = staticCastPtr<ModelNode*>(parent);

		--index;
		trf = mnode->m_transforms[index];
	}
}

//==============================================================================
void ModelPatchNode::updateInstanceSpatials(
	const MoveComponent* instanceMoves[],
	U32 instanceMovesCount)
{
	Bool fullUpdate = false;

	const U oldSize = m_spatials.getSize();
	const U newSize = instanceMovesCount;

	if(oldSize < newSize)
	{
		// We need to add spatials

		fullUpdate = true;

		m_spatials.resize(getSceneAllocator(), newSize);

		U diff = newSize - oldSize;
		U index = oldSize;
		while(diff-- != 0)
		{
			ObbSpatialComponent* newSpatial = getSceneAllocator().
				newInstance<ObbSpatialComponent>(this);

			addComponent(newSpatial);
			m_spatials[index++] = newSpatial;
		}
	}
	else if(oldSize > newSize)
	{
		// Need to remove spatials

		fullUpdate = true;

		// TODO
		ANKI_ASSERT(0 && "TODO");
	}

	U count = newSize;
	const Obb& localBoundingShape = m_modelPatch->getBoundingShape();
	while(count-- != 0)
	{
		ObbSpatialComponent& sp = *m_spatials[count];
		ANKI_ASSERT(count < instanceMovesCount);
		const MoveComponent& inst = *instanceMoves[count];

		if(sp.getTimestamp() < inst.getTimestamp() || fullUpdate)
		{
			sp.m_obb = localBoundingShape.getTransformed(
				inst.getWorldTransform());

			sp.markForUpdate();
			sp.setSpatialOrigin(inst.getWorldTransform().getOrigin());
		}
	}
}

//==============================================================================
// ModelMoveFeedbackComponent                                                  =
//==============================================================================

/// Feedback component.
class ModelMoveFeedbackComponent: public SceneComponent
{
public:
	ModelMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponent::Type::NONE, node)
	{}

	ANKI_USE_RESULT Error update(
		SceneNode& node, F32, F32, Bool& updated)
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			ModelNode& mnode = static_cast<ModelNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(SceneGraph* scene)
	: SceneNode(scene)
	, m_transformsTimestamp(0)
{}

//==============================================================================
ModelNode::~ModelNode()
{
	m_modelPatches.destroy(getSceneAllocator());
	m_transforms.destroy(getSceneAllocator());
}

//==============================================================================
Error ModelNode::create(const CString& name, const CString& modelFname)
{
	ANKI_CHECK(SceneNode::create(name));

	SceneComponent* comp;

	ANKI_CHECK(m_model.load(modelFname, &getResourceManager()));

	m_modelPatches.create(
		getSceneAllocator(), m_model->getModelPatches().getSize(), nullptr);

	U count = 0;
	auto it = m_model->getModelPatches().getBegin();
	auto end = m_model->getModelPatches().getEnd();
	for(; it != end; it++)
	{
		ModelPatchNode* mpn;
		StringAuto nname(getFrameAllocator());
		nname.sprintf("%s_%d", &name[0], count);
		ANKI_CHECK(getSceneGraph().newSceneNode(nname.toCString(), mpn, *it));

		m_modelPatches[count++] = mpn;
		addChild(mpn);
	}

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().newInstance<ModelMoveFeedbackComponent>(this);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

//==============================================================================
Error ModelNode::frameUpdate(F32, F32)
{
	// Gather the move components of the instances
	DArrayAuto<const MoveComponent*> instanceMoves(getFrameAllocator());
	U instanceMovesCount = 0;
	Timestamp instancesTimestamp = 0;

	instanceMoves.create(64);

	Error err = visitChildren([&](SceneNode& sn) -> Error
	{
		if(sn.tryGetComponent<InstanceComponent>())
		{
			MoveComponent& move = sn.getComponent<MoveComponent>();

			instanceMoves[instanceMovesCount++] = &move;

			instancesTimestamp =
				max(instancesTimestamp, move.getTimestamp());
		}

		return ErrorCode::NONE;
	});
	(void)err;

	// If having instances
	if(instanceMovesCount > 0)
	{
		Bool fullUpdate = false;

		if(instanceMovesCount != m_transforms.getSize())
		{
			fullUpdate = true;
			m_transforms.resize(getSceneAllocator(), instanceMovesCount);
		}

		if(fullUpdate || m_transformsTimestamp < instancesTimestamp)
		{
			m_transformsTimestamp = instancesTimestamp;

			for(U i = 0; i < instanceMovesCount; ++i)
			{
				m_transforms[i] = instanceMoves[i]->getWorldTransform();
			}
		}

		// Update children
		auto it = m_modelPatches.getBegin();
		auto end = m_modelPatches.getEnd();
		for(; it != end; ++it)
		{
			(*it)->updateInstanceSpatials(
				&instanceMoves[0], instanceMovesCount);
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
void ModelNode::onMoveComponentUpdate(MoveComponent& move)
{
	// Inform the children about the moves
	for(ModelPatchNode* child : m_modelPatches)
	{
		child->m_obb = child->m_modelPatch->getBoundingShape().getTransformed(
			move.getWorldTransform());

		SpatialComponent& sp = child->getComponent<SpatialComponent>();
		sp.markForUpdate();
		sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	}
}

} // end namespace anki
