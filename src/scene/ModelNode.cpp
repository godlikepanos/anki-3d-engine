// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
	:	RenderComponent(node),
		m_node(node)
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
:	SceneNode(scene)
{}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{
	m_spatials.destroy(getSceneAllocator());
}

//==============================================================================
Error ModelPatchNode::create(const CString& name, 
	const ModelPatchBase* modelPatch)
{
	ANKI_ASSERT(modelPatch);
	Error err = SceneNode::create(name);
	if(err) return err;

	m_modelPatch = modelPatch;

	// Spatial component
	SceneComponent* comp = getSceneAllocator().newInstance<SpatialComponent>(
		this, &m_obb);
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY;

	err = addComponent(comp, true);
	if(err) return err;

	// Render component
	RenderComponent* rcomp = 
		getSceneAllocator().newInstance<ModelPatchRenderComponent>(this);
	comp = rcomp;
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY;

	err = addComponent(comp, true);
	if(err) return err;

	err = rcomp->create();

	return err;
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

	GlCommandBufferHandle vertJobs;
	GlPipelineHandle ppline;

	Error err = m_modelPatch->getRenderingDataSub(
		data.m_key, vertJobs, ppline, 
		nullptr, 0,
		indicesCountArray, indicesOffsetArray, drawcallCount);

	if(!err)
	{
		// Cannot accept multi-draw
		ANKI_ASSERT(drawcallCount == 1);

		// Set jobs
		ppline.bind(data.m_jobs);
		data.m_jobs.pushBackOtherCommandBuffer(vertJobs);

		// Drawcall
		U32 offset = indicesOffsetArray[0] / sizeof(U16);
		data.m_jobs.drawElements(
			data.m_key.m_tessellation ? GL_PATCHES : GL_TRIANGLES,
			sizeof(U16),
			indicesCountArray[0],
			instancesCount,
			offset);
	}

	return err;
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
Error ModelPatchNode::updateInstanceSpatials(
	const MoveComponent* instanceMoves[],
	U32 instanceMovesCount)
{
	Error err = ErrorCode::NONE;
	Bool fullUpdate = false;

	const U oldSize = m_spatials.getSize();
	const U newSize = instanceMovesCount;

	if(oldSize < newSize)
	{
		// We need to add spatials
		
		fullUpdate = true;
		
		err = m_spatials.resize(getSceneAllocator(), newSize);
		if(err)	return err;

		U diff = newSize - oldSize;
		U index = oldSize;
		while(diff-- != 0)
		{
			ObbSpatialComponent* newSpatial = getSceneAllocator().
				newInstance<ObbSpatialComponent>(this);
			if(newSpatial == nullptr) return ErrorCode::OUT_OF_MEMORY;

			err = addComponent(newSpatial);
			if(err)	return err;

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

	return err;
}

//==============================================================================
// ModelMoveFeedbackComponent                                                  =
//==============================================================================

/// Feedback component.
class ModelMoveFeedbackComponent: public SceneComponent
{
public:
	ModelMoveFeedbackComponent(SceneNode* node)
	:	SceneComponent(SceneComponent::Type::NONE, node)
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
// ModelBodyFeedbackComponent                                                  =
//==============================================================================

/// Body feedback component.
class ModelBodyFeedbackComponent: public SceneComponent
{
public:
	ModelBodyFeedbackComponent(SceneNode* node)
	:	SceneComponent(SceneComponent::Type::NONE, node)
	{}

	ANKI_USE_RESULT Error update(
		SceneNode& node, F32, F32, Bool& updated)
	{
		updated = false;

		BodyComponent& bodyc = node.getComponent<BodyComponent>();

		if(bodyc.getTimestamp() == node.getGlobalTimestamp())
		{
			MoveComponent& move = node.getComponent<MoveComponent>();
			move.setLocalTransform(bodyc.getTransform());
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(SceneGraph* scene)
: 	SceneNode(scene),
	m_transformsTimestamp(0)
{}

//==============================================================================
ModelNode::~ModelNode()
{
	m_modelPatches.destroy(getSceneAllocator());
	m_transforms.destroy(getSceneAllocator());

	if(m_body)
	{
		getSceneAllocator().deleteInstance(m_body);
	}
}

//==============================================================================
Error ModelNode::create(const CString& name, const CString& modelFname)
{
	Error err = ErrorCode::NONE;

	err = SceneNode::create(name);
	if(err) return err;

	SceneComponent* comp;

	err = m_model.load(modelFname, &getResourceManager());
	if(err) return err;

	err = m_modelPatches.create(
		getSceneAllocator(), m_model->getModelPatches().getSize(), nullptr);
	if(err) return err;

	U count = 0;
	auto it = m_model->getModelPatches().getBegin();
	auto end = m_model->getModelPatches().getEnd();
	for(; it != end; it++)
	{
		ModelPatchNode* mpn;
		err = getSceneGraph().newSceneNode(CString(), mpn, *it);
		if(err) return err;

		m_modelPatches[count++] = mpn;
		err = addChild(mpn);
		if(err) return err;
	}

	// Load rigid body
	const PhysicsCollisionShape* shape = m_model->getPhysicsCollisionShape();
	if(shape != nullptr)
	{
		PhysicsBody::Initializer init;
		init.m_mass = 1.0;
		init.m_shape = shape;

		m_body = 
			getSceneGraph()._getPhysicsWorld().newBody<PhysicsBody>(init);
		if(m_body == nullptr) return ErrorCode::OUT_OF_MEMORY;

		// Body component
		comp = getSceneAllocator().newInstance<BodyComponent>(this, m_body);
		if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY;

		err = addComponent(comp, true);
		if(err) return err;

		// Feedback component
		comp = 
			getSceneAllocator().newInstance<ModelBodyFeedbackComponent>(this);
		if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY;

		err = addComponent(comp, true);
		if(err) return err;
	}

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY;

	err = addComponent(comp, true);
	if(err) return err;

	// Feedback component
	comp = getSceneAllocator().newInstance<ModelMoveFeedbackComponent>(this);
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY;

	err = addComponent(comp, true);
	if(err) return err;

	return err;
}

//==============================================================================
Error ModelNode::frameUpdate(F32, F32)
{
	Error err = ErrorCode::NONE;

	// Gather the move components of the instances
	SceneFrameDArrayAuto<const MoveComponent*> instanceMoves(
		getSceneFrameAllocator());
	U instanceMovesCount = 0;
	Timestamp instancesTimestamp = 0;

	err = instanceMoves.create(64);
	if(err)	return err;

	err = visitChildren([&](SceneNode& sn) -> Error
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

	// If having instances
	if(instanceMovesCount > 0)
	{
		Bool fullUpdate = false;

		if(instanceMovesCount != m_transforms.getSize())
		{
			fullUpdate = true;
			err = m_transforms.resize(getSceneAllocator(), instanceMovesCount);
			if(err) return err;
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
			err = (*it)->updateInstanceSpatials(
				&instanceMoves[0], instanceMovesCount);
			return err;
		}
	}

	return err;
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
