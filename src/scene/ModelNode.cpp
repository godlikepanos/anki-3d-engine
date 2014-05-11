#include "anki/scene/ModelNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/InstanceNode.h"
#include "anki/scene/Misc.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(
	const char* name, SceneGraph* scene,
	const ModelPatchBase* modelPatch)
	:	SceneNode(name, scene),
		RenderComponent(this),
		SpatialComponent(this), 
		m_modelPatch(modelPatch)
{
	addComponent(static_cast<RenderComponent*>(this));
	addComponent(static_cast<SpatialComponent*>(this));

	RenderComponent::init();
}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{}

//==============================================================================
void ModelPatchNode::buildRendering(RenderingBuildData& data)
{
	// That will not work on multi-draw and instanced at the same time. Make
	// sure that there is no multi-draw anywhere
	ANKI_ASSERT(m_modelPatch->getSubMeshesCount() == 0);

	auto instancesCount = data.m_subMeshIndicesCount;

	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS> indicesCountArray;
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS> indicesOffsetArray;
	U32 drawcallCount;

	GlJobChainHandle vertJobs;
	GlProgramPipelineHandle ppline;

	m_modelPatch->getRenderingDataSub(
		data.m_key, vertJobs, ppline, 
		nullptr, 0,
		indicesCountArray, indicesOffsetArray, drawcallCount);

	// Cannot accept multi-draw
	ANKI_ASSERT(drawcallCount == 1);

	// Drawcall
	U32 offset = indicesOffsetArray[0] / sizeof(U16);
	GlDrawcallElements dc(
		data.m_key.m_tessellation ? GL_PATCHES : GL_TRIANGLES,
		sizeof(U16),
		indicesCountArray[0],
		instancesCount,
		offset);

	// Set jobs
	ppline.bind(data.m_jobs);
	data.m_jobs.pushBackOtherJobChain(vertJobs);
	dc.draw(data.m_jobs);
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
void ModelPatchNode::frameUpdate(F32, F32, SceneNode::UpdateType uptype)
{
	if(uptype != SceneNode::ASYNC_UPDATE)
	{
		return;
	}

	SceneNode* parent = &getParent()->downCast<SceneNode>();
	ANKI_ASSERT(parent);

	// Update first OBB
	const MoveComponent& parentMove = parent->getComponent<MoveComponent>();
	if(parentMove.getTimestamp() == getGlobTimestamp())
	{
		m_obb = m_modelPatch->getBoundingShape().getTransformed(
			parentMove.getWorldTransform());

		SpatialComponent::markForUpdate();
	}

	// Get the move components of the instances of the parent
	SceneFrameVector<MoveComponent*> instanceMoves(getSceneFrameAllocator());
	Timestamp instancesTimestamp = 0;
	parent->visitThisAndChildren<SceneNode>([&](SceneNode& sn)
	{		
		if(sn.tryGetComponent<InstanceComponent>())
		{
			MoveComponent& move = sn.getComponent<MoveComponent>();

			instanceMoves.push_back(&move);

			instancesTimestamp = 
				std::max(instancesTimestamp, move.getTimestamp());
		}
	});

	// Instancing?
	if(instanceMoves.size() != 0)
	{
		Bool needsUpdate = false;

		// Get the spatials and their update time
		SceneFrameVector<ObbSpatialComponent*> 
			spatials(getSceneFrameAllocator());

		Timestamp spatialsTimestamp = 0;
		U count = 0;

		iterateComponentsOfType<SpatialComponent>([&](SpatialComponent& sp)
		{
			// Skip the first
			if(count != 0)	
			{
				ObbSpatialComponent* msp = 
					staticCastPtr<ObbSpatialComponent*>(&sp);
		
				spatialsTimestamp = 
					std::max(spatialsTimestamp, msp->getTimestamp());

				spatials.push_back(msp);
			}
			++count;
		});

		// If we need to add spatials
		if(spatials.size() < instanceMoves.size())
		{
			needsUpdate = true;
			U diff = instanceMoves.size() - spatials.size();

			while(diff-- != 0)
			{
				ObbSpatialComponent* newSpatial = getSceneAllocator().
					newInstance<ObbSpatialComponent>(this);

				addComponent(newSpatial);

				spatials.push_back(newSpatial);
			}
		}
		// If we need to remove spatials
		else if(spatials.size() > instanceMoves.size())
		{
			needsUpdate = true;

			// TODO
			ANKI_ASSERT(0 && "TODO");
		}
		
		// Now update all spatials if needed
		if(needsUpdate || spatialsTimestamp < instancesTimestamp)
		{
			for(U i = 0; i < spatials.size(); i++)
			{
				ObbSpatialComponent* sp = spatials[i];

				sp->obb = m_modelPatch->getBoundingShape().getTransformed(
					instanceMoves[i]->getWorldTransform());

				sp->markForUpdate();
			}
		}
	} // end instancing
}

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(
	const char* name, SceneGraph* scene,
	const char* modelFname)
	: 	SceneNode(name, scene),
		MoveComponent(this),
		m_transforms(getSceneAllocator()),
		m_transformsTimestamp(0)
{
	addComponent(static_cast<MoveComponent*>(this));

	m_model.load(modelFname);

	for(const ModelPatchBase* patch : m_model->getModelPatches())
	{
		ModelPatchNode* mpn = 
			getSceneGraph().newSceneNode<ModelPatchNode>(nullptr, patch);

		SceneObject::addChild(mpn);
	}

	// Load rigid body
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
}

//==============================================================================
ModelNode::~ModelNode()
{
	RigidBody* body = tryGetComponent<RigidBody>();
	if(body)
	{
		getSceneGraph().getPhysics().deletePhysicsObject(body);
	}
}

//==============================================================================
void ModelNode::frameUpdate(F32, F32, SceneNode::UpdateType uptype)
{
	if(uptype != SceneNode::ASYNC_UPDATE)
	{
		return;
	}

	// Get the move components of the instances of the parent
	SceneFrameVector<MoveComponent*> instanceMoves(getSceneFrameAllocator());
	Timestamp instancesTimestamp = 0;
	SceneObject::visitThisAndChildren<SceneNode>([&](SceneNode& sn)
	{		
		if(sn.tryGetComponent<InstanceComponent>())
		{
			MoveComponent& move = sn.getComponent<MoveComponent>();

			instanceMoves.push_back(&move);

			instancesTimestamp = 
				std::max(instancesTimestamp, move.getTimestamp());
		}
	});

	// If instancing
	if(instanceMoves.size() != 0)
	{
		Bool transformsNeedUpdate = false;

		if(instanceMoves.size() != m_transforms.size())
		{
			transformsNeedUpdate = true;
			m_transforms.resize(instanceMoves.size());
		}

		if(transformsNeedUpdate || m_transformsTimestamp < instancesTimestamp)
		{
			m_transformsTimestamp = instancesTimestamp;

			for(U i = 0; i < instanceMoves.size(); i++)
			{
				m_transforms[i] = instanceMoves[i]->getWorldTransform();
			}
		}
	}
}

} // end namespace anki
