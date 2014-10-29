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
ModelPatchNode::ModelPatchNode(
	SceneGraph* scene,
	const ModelPatchBase* modelPatch)
:	SceneNode(scene),
	RenderComponent(this),
	SpatialComponent(this), 
	m_modelPatch(modelPatch)
{}

//==============================================================================
Error ModelPatchNode::create(const CString& name)
{
	err = SceneNode::create(name, 2);

	if(!err)
	{
		addComponent(static_cast<RenderComponent*>(this));
		addComponent(static_cast<SpatialComponent*>(this));

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
	if(err)
	{
		return err;
	}

	// Cannot accept multi-draw
	ANKI_ASSERT(drawcallCount == 1);

	// Set jobs
	ppline.bind(data.m_jobs);
	err = data.m_jobs.pushBackOtherCommandBuffer(vertJobs);
	if(err)
	{
		return err;
	}
	
	// Drawcall
	U32 offset = indicesOffsetArray[0] / sizeof(U16);
	err = data.m_jobs.drawElements(
		data.m_key.m_tessellation ? GL_PATCHES : GL_TRIANGLES,
		sizeof(U16),
		indicesCountArray[0],
		instancesCount,
		offset);

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
		ANKI_ASSERT(index < mnode->m_transforms.size());
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

			addComponent(newSpatial);

			m_spatials[index++] = newSpatial;
		}
	}
	else if(m_spatials.size() > instanceMoves.size())
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
ModelNode::ModelNode(
	const CString& name, SceneGraph* scene,
	const CString& modelFname)
: 	SceneNode(name, scene),
	MoveComponent(this),
	m_modelPatches(getSceneAllocator()),
	m_transforms(getSceneAllocator()),
	m_transformsTimestamp(0)
{
	addComponent(static_cast<MoveComponent*>(this));

	m_model.load(modelFname, &getResourceManager());
	m_modelPatches.reserve(m_model->getModelPatches().getSize());

	for(const ModelPatchBase* patch : m_model->getModelPatches())
	{
		ModelPatchNode* mpn = 
			getSceneGraph().newSceneNode<ModelPatchNode>(CString(), patch);

		m_modelPatches.push_back(mpn);

		SceneObject::addChild(mpn);
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
}

//==============================================================================
ModelNode::~ModelNode()
{
#if 0
	RigidBody* body = tryGetComponent<RigidBody>();
	if(body)
	{
		getSceneGraph().getPhysics().deletePhysicsObject(body);
	}
#endif
}

//==============================================================================
void ModelNode::frameUpdate(F32, F32)
{
	// Gather the move components of the instances
	SceneFrameVector<MoveComponent*> instanceMoves(getSceneFrameAllocator());
	Timestamp instancesTimestamp = 0;
	SceneObject::visitChildren([&](SceneObject& obj) -> Error
	{
		if(obj.getType() == SceneNode::getClassType())
		{
			SceneNode& sn = obj.downCast<SceneNode>();
			if(sn.tryGetComponent<InstanceComponent>())
			{
				MoveComponent& move = sn.getComponent<MoveComponent>();

				instanceMoves.push_back(&move);

				instancesTimestamp = 
					std::max(instancesTimestamp, move.getTimestamp());
			}
		}

		return ErrorCode::NONE;
	});

	// If having instances
	if(instanceMoves.size() != 0)
	{
		Bool fullUpdate = false;

		if(instanceMoves.size() != m_transforms.size())
		{
			fullUpdate = true;
			m_transforms.resize(instanceMoves.size());
		}

		if(fullUpdate || m_transformsTimestamp < instancesTimestamp)
		{
			m_transformsTimestamp = instancesTimestamp;

			for(U i = 0; i < instanceMoves.size(); i++)
			{
				m_transforms[i] = instanceMoves[i]->getWorldTransform();
			}
		}

		// Update children
		for(ModelPatchNode* child : m_modelPatches)
		{
			child->updateInstanceSpatials(instanceMoves);
		}
	}
}

//==============================================================================
void ModelNode::onMoveComponentUpdate(SceneNode&, F32, F32)
{
	// Inform the children about the moves
	for(ModelPatchNode* child : m_modelPatches)
	{
		child->m_obb = child->m_modelPatch->getBoundingShape().getTransformed(
			getWorldTransform());

		child->SpatialComponent::markForUpdate();
	}
}

} // end namespace anki
