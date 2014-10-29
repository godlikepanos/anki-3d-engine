// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/ModelNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/InstanceNode.h"
#include "anki/scene/Misc.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(SceneGraph* scene)
:	SceneNode(scene),
	RenderComponent(this),
	SpatialComponent(this)
{}

//==============================================================================
Error ModelPatchNode::create(const CString& name, 
	const ModelPatchBase* modelPatch)
{
	ANKI_ASSERT(modelPatch);
	Error err = SceneNode::create(name);

	m_modelPatch = modelPatch;

	if(!err)
	{
		err = addComponent(static_cast<RenderComponent*>(this));
	}

	if(!err)
	{
		err = addComponent(static_cast<SpatialComponent*>(this));
	}

	if(!err)
	{
		err = RenderComponent::create();
	}

	return err;
}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{
	m_spatials.destroy(getSceneAllocator());
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
	GlProgramPipelineHandle ppline;

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
	SceneNode* parent = &getParent()->downCast<SceneNode>();
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
		SceneNode* parent = &getParent()->downCast<SceneNode>();
		ANKI_ASSERT(parent);
		ModelNode* mnode = staticCastPtr<ModelNode*>(parent);

		--index;
		ANKI_ASSERT(index < mnode->m_transforms.getSize());
		trf = mnode->m_transforms[index];
	}
}

//==============================================================================
Error ModelPatchNode::updateInstanceSpatials(
	const SceneFrameDArray<MoveComponent*>& instanceMoves)
{
	Error err = ErrorCode::NONE;
	Bool fullUpdate = false;

	const U oldSize = m_spatials.getSize();
	const U newSize = instanceMoves.getSize();

	if(oldSize < newSize)
	{
		// We need to add spatials
		
		fullUpdate = true;
		
		err = m_spatials.resize(getSceneAllocator(), newSize);
		if(err)
		{
			return err;
		}

		U diff = newSize - oldSize;
		U index = oldSize;
		while(diff-- != 0)
		{
			ObbSpatialComponent* newSpatial = getSceneAllocator().
				newInstance<ObbSpatialComponent>(this);
			if(newSpatial == nullptr)
			{
				return ErrorCode::OUT_OF_MEMORY;
			}

			err = addComponent(newSpatial);
			if(err)
			{
				return err;
			}

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
	while(count-- != 0)
	{
		ObbSpatialComponent& sp = *m_spatials[count];
		const MoveComponent& inst = *instanceMoves[count];

		if(sp.getTimestamp() < inst.getTimestamp() || fullUpdate)
		{
			sp.m_obb = m_modelPatch->getBoundingShape().getTransformed(
				inst.getWorldTransform());

			sp.markForUpdate();
		}
	}

	return err;
}

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(SceneGraph* scene)
: 	SceneNode(scene),
	MoveComponent(this),
	m_transformsTimestamp(0)
{}

//==============================================================================
ModelNode::~ModelNode()
{
	m_modelPatches.destroy(getSceneAllocator());
	m_transforms.destroy(getSceneAllocator());
#if 0
	RigidBody* body = tryGetComponent<RigidBody>();
	if(body)
	{
		getSceneGraph().getPhysics().deletePhysicsObject(body);
	}
#endif
}

//==============================================================================
Error ModelNode::create(const CString& name, const CString& modelFname)
{
	Error err = ErrorCode::NONE;

	err = SceneNode::create(name);

	if(!err)
	{
		err = addComponent(static_cast<MoveComponent*>(this));
	}

	if(!err)
	{
		err = m_model.load(modelFname, &getResourceManager());
	}

	if(!err)
	{
		err = m_modelPatches.create(
			getSceneAllocator(), m_model->getModelPatches().getSize());
	}

	if(!err)
	{
		U count = 0;
		auto it = m_model->getModelPatches().getBegin();
		auto end = m_model->getModelPatches().getEnd();
		for(; !err && it != end; it++)
		{
			ModelPatchNode* mpn;
			err = getSceneGraph().newSceneNode(CString(), mpn, *it);

			if(!err)
			{
				m_modelPatches[count++] = mpn;

				err = SceneObject::addChild(mpn);
			}
		}
	}

	// Load rigid body
#if 0
	if(m_model->getCollisionShape() != nullptr)
	{
		RigidBody::Initializer init;
		init.mass = 1.0;
		init.shape = const_cast<btCollisionShape*>(m_model->getCollisionShape());
		init.node = this;

		RigidBody* body;
		
		getSceneGraph().getPhysics().newPhysicsObject<RigidBody>(
			body, init);

		addComponent(static_cast<RigidBody*>(body));
	}
#endif

	return err;
}

//==============================================================================
Error ModelNode::frameUpdate(F32, F32)
{
	Error err = ErrorCode::NONE;

	// Gather the move components of the instances
	SceneFrameDArray<MoveComponent*> instanceMoves;
	U instanceMovesCount = 0;
	Timestamp instancesTimestamp = 0;

	err = instanceMoves.create(getSceneFrameAllocator(), 64);
	if(err)
	{
		return err;
	}

	err = SceneObject::visitChildren([&](SceneObject& obj) -> Error
	{
		if(obj.getType() == SceneNode::getClassType())
		{
			SceneNode& sn = obj.downCast<SceneNode>();
			if(sn.tryGetComponent<InstanceComponent>())
			{
				MoveComponent& move = sn.getComponent<MoveComponent>();

				instanceMoves[instanceMovesCount++] = &move;

				instancesTimestamp = 
					std::max(instancesTimestamp, move.getTimestamp());
			}
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
		}

		if(!err && (fullUpdate || m_transformsTimestamp < instancesTimestamp))
		{
			m_transformsTimestamp = instancesTimestamp;

			U count = 0;
			for(const MoveComponent* instanceMove : instanceMoves)
			{
				m_transforms[count++] = instanceMove->getWorldTransform();
			}
		}

		// Update children
		if(!err)
		{
			auto it = m_modelPatches.getBegin();
			auto end = m_modelPatches.getEnd();
			for(; it != end && !err; ++it)
			{
				err = (*it)->updateInstanceSpatials(instanceMoves);
			}
		}
	}

	instanceMoves.destroy(getSceneFrameAllocator());

	return err;
}

//==============================================================================
Error ModelNode::onMoveComponentUpdate(SceneNode&, F32, F32)
{
	// Inform the children about the moves
	for(ModelPatchNode* child : m_modelPatches)
	{
		child->m_obb = child->m_modelPatch->getBoundingShape().getTransformed(
			getWorldTransform());

		child->SpatialComponent::markForUpdate();
	}

	return ErrorCode::NONE;
}

} // end namespace anki
