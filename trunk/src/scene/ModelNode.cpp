#include "anki/scene/ModelNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/InstanceNode.h"
#include "anki/scene/Misc.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/gl/Drawcall.h"

namespace anki {

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(
	const char* name, SceneGraph* scene,
	const ModelPatchBase* modelPatch_)
	:	SceneNode(name, scene),
		RenderComponent(this),
		SpatialComponent(this), 
		modelPatch(modelPatch_)
{
	addComponent(static_cast<RenderComponent*>(this));
	addComponent(static_cast<SpatialComponent*>(this));

	RenderComponent::init();
}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{}

//==============================================================================
void ModelPatchNode::getRenderingData(
	const PassLodKey& key, 
	const U8* subMeshIndicesArray, U subMeshIndicesCount,
	const Vao*& vao, const ShaderProgram*& prog,
	Drawcall& dc)
{
	dc.primitiveType = GL_TRIANGLES;
	dc.indicesType = GL_UNSIGNED_SHORT;

	U spatialsCount = 0;
	iterateComponentsOfType<SpatialComponent>([&](SpatialComponent&)
	{
		++spatialsCount;
	});

	dc.instancesCount = std::min(spatialsCount, subMeshIndicesCount);

	modelPatch->getRenderingDataSub(key, vao, prog, 
		subMeshIndicesArray, subMeshIndicesCount, 
		dc.countArray, dc.offsetArray, dc.drawCount);
}

//==============================================================================
void ModelPatchNode::getRenderWorldTransform(U index, Transform& trf)
{
	SceneNode* parent = &getParent()->downCast<SceneNode>();
	ANKI_ASSERT(parent);
	MoveComponent& move = parent->getComponent<MoveComponent>();

	if(index == 0)
	{
		// NO instancing
		trf = move.getWorldTransform();
	}
	else
	{
		// Instancing
		SceneNode* parent = &getParent()->downCast<SceneNode>();
		ANKI_ASSERT(parent);
		ModelNode* mnode = staticCastPtr<ModelNode*>(parent);

		--index;
		ANKI_ASSERT(index < mnode->transforms.size());
		trf = mnode->transforms[index];
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
		obb = modelPatch->getBoundingShape().getTransformed(
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

				sp->obb = modelPatch->getBoundingShape().getTransformed(
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
		transforms(getSceneAllocator()),
		transformsTimestamp(0)
{
	addComponent(static_cast<MoveComponent*>(this));

	model.load(modelFname);

	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		ModelPatchNode* mpn;
		getSceneGraph().newSceneNode(mpn, nullptr, patch);

		SceneObject::addChild(mpn);
	}

	// Load rigid body
	if(model->getCollisionShape() != nullptr)
	{
		RigidBody::Initializer init;
		init.mass = 1.0;
		init.shape = const_cast<btCollisionShape*>(model->getCollisionShape());
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

		if(instanceMoves.size() != transforms.size())
		{
			transformsNeedUpdate = true;
			transforms.resize(instanceMoves.size());
		}

		if(transformsNeedUpdate || transformsTimestamp < instancesTimestamp)
		{
			transformsTimestamp = instancesTimestamp;

			for(U i = 0; i < instanceMoves.size(); i++)
			{
				transforms[i] = instanceMoves[i]->getWorldTransform();
			}
		}
	}
}

} // end namespace anki
