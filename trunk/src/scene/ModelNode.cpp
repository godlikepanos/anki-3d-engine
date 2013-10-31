#include "anki/scene/ModelNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include "anki/physics/RigidBody.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
// ModelPatchNodeInstance                                                      =
//==============================================================================

//==============================================================================
ModelPatchNodeInstance::ModelPatchNodeInstance(
	const char* name, SceneGraph* scene, // Scene
	ModelPatchNode* modelPatchNode_) // Self
	:	SceneNode(name, scene),
		MoveComponent(this),
		SpatialComponent(this, &obb),
		modelPatchNode(modelPatchNode_)
{
	sceneNodeProtected.moveC = this;

	// Dont mark it as spatial because it's sub-spatial and don't want to 
	// be updated by the scene
	sceneNodeProtected.spatialC = nullptr;

	ANKI_ASSERT(modelPatchNode);
}

//==============================================================================
void ModelPatchNodeInstance::moveUpdate()
{
	ANKI_ASSERT(modelPatchNode);

	// Update the obb of self
	obb = modelPatchNode->modelPatch->getBoundingShape().getTransformed(
		getWorldTransform());
	SpatialComponent::markForUpdate();

	// If this instance is the last update the parent's collision shape
	ANKI_ASSERT(modelPatchNode->instances.size() > 0);
	if(this == modelPatchNode->instances.back())
	{
		modelPatchNode->updateSpatialCs();
	}
}

//==============================================================================
// ModelPatchNode                                                              =
//==============================================================================

//==============================================================================
ModelPatchNode::ModelPatchNode(
	const char* name, SceneGraph* scene,
	const ModelPatchBase* modelPatch_, U instancesCount)
	:	SceneNode(name, scene),
		MoveComponent(this, MoveComponent::MF_IGNORE_LOCAL_TRANSFORM),
		RenderComponent(this),
		SpatialComponent(this, &obb), 
		modelPatch(modelPatch_),
		instances(getSceneAllocator()),
		transforms(getSceneAllocator())
{
	sceneNodeProtected.moveC = this;
	sceneNodeProtected.renderC = this;
	sceneNodeProtected.spatialC = this;

	RenderComponent::init();

	// Create the instances as ModelPatchNodeInstance
	if(instancesCount > 1)
	{
		instances.resize(instancesCount, nullptr);
		transforms.resize(instancesCount, Transform::getIdentity());

		Vec3 pos = Vec3(0.0);

		for(U i = 0; i < instancesCount; i++)
		{
			ModelPatchNodeInstance* instance;
			getSceneGraph().newSceneNode(instance, nullptr, this);

			//MoveComponent::addChild(instance);

			instance->setLocalOrigin(pos);
			pos.x() += 2.0;

			SpatialComponent::addChild(instance);
			instances[i] = instance;
		}
	}
}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{
	for(ModelPatchNodeInstance* instance : instances)
	{
		getSceneGraph().deleteSceneNode(instance);
	}
}

//==============================================================================
const Transform* ModelPatchNode::getRenderWorldTransforms()
{
	if(transforms.size() == 0)
	{
		// NO instancing
		return &getWorldTransform();
	}
	else
	{
		// Instancing

		ANKI_ASSERT(transforms.size() == instances.size());

		// Set the transforms buffer
		for(U i = 0; i < instances.size(); i++)
		{
			transforms[i] = instances[i]->getWorldTransform();
		}

		return &transforms[0];
	}
}

//==============================================================================
void ModelPatchNode::moveUpdate()
{
	if(instances.size() == 0)
	{
		// NO instancing

		obb = modelPatch->getBoundingShape().getTransformed(
			getWorldTransform());

		SpatialComponent::markForUpdate();
	}
	else
	{
		// Instancing
		
		// Do nothing. You cannot update the obb because the instances have
		// not been updated their CSs yet. The update will be triggered by the 
		// last instance.
	}
}

//==============================================================================
void ModelPatchNode::updateSpatialCs()
{
	ANKI_ASSERT(instances.size() > 0);

	obb = instances[0]->obb;

	for(U i = 1; i < instances.size(); i++)
	{
		ANKI_ASSERT(instances[i]);

		obb = obb.getCompoundShape(instances[i]->obb);
	}

	SpatialComponent::markForUpdate();
}

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(
	const char* name, SceneGraph* scene,
	const char* modelFname, U instances)
	: 	SceneNode(name, scene),
		MoveComponent(this),
		patches(getSceneAllocator())
{
	sceneNodeProtected.moveC = this;

	model.load(modelFname);

	patches.reserve(model->getModelPatches().size());

	U i = 0;
	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		ModelPatchNode* mpn;
		getSceneGraph().newSceneNode(mpn, nullptr, patch, instances);

		MoveComponent::addChild(mpn);

		patches.push_back(mpn);
		++i;
	}

	// Load rigid body
	if(model->getCollisionShape() != nullptr)
	{
		RigidBody::Initializer init;
		init.mass = 1.0;
		init.shape = const_cast<btCollisionShape*>(model->getCollisionShape());
		init.startTrf.getOrigin().y() = 20.0;

		RigidBody* body;
		
		getSceneGraph().getPhysics().newPhysicsObject<RigidBody>(
			body, init);

		sceneNodeProtected.rigidBodyC = body;
	}
}

//==============================================================================
ModelNode::~ModelNode()
{
	for(ModelPatchNode* patch : patches)
	{
		getSceneGraph().deleteSceneNode(patch);
	}

	if(sceneNodeProtected.rigidBodyC)
	{
		getSceneGraph().getPhysics().deletePhysicsObject(
			sceneNodeProtected.rigidBodyC);
	}
}

//==============================================================================
void ModelNode::setInstanceLocalTransform(U instanceIndex, const Transform& trf)
{
	ANKI_ASSERT(patches.size() > 0);

	if(patches[0]->instances.size() > 0)
	{
		ANKI_ASSERT(instanceIndex < patches[0]->instances.size());

		for(ModelPatchNode* patch : patches)
		{
			patch->instances[instanceIndex]->setLocalTransform(trf);
		}
	}
	else
	{
		ANKI_ASSERT(instanceIndex == 0);

		setLocalTransform(trf);
	}
}

} // end namespace anki
