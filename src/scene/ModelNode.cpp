#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"

namespace anki {

//==============================================================================
// ModelPatchNodeInstance                                                      =
//==============================================================================

//==============================================================================
ModelPatchNodeInstance::ModelPatchNodeInstance(
	const char* name, SceneGraph* scene, SceneNode* parent, // Scene
	U32 movableFlags, // Movable
	const ModelPatchBase* modelPatchResource) // Self
	:	SceneNode(name, scene, parent),
		Movable(movableFlags, this),
		Spatial(&obb, getSceneAllocator()),
		modelPatch(modelPatchResource)
{
	sceneNodeProtected.movable = this;

	// Dont mark it as spatial because it's sub-spatial and don't want to 
	// be updated by the scene
	sceneNodeProtected.spatial = nullptr;

	ANKI_ASSERT(modelPatch);
}

//==============================================================================
void ModelPatchNodeInstance::movableUpdate()
{
	ANKI_ASSERT(modelPatch);

	// Update the obb of self
	obb = modelPatch->getBoundingShape().getTransformed(
		getWorldTransform());
	spatialMarkForUpdate();

	// If this instance is the last update the parent's collision shape
	SceneNode* parentNode = getParent();
	ANKI_ASSERT(parentNode);

	ModelPatchNode* modelPatchNode = 
#if ANKI_DEBUG
		dynamic_cast<ModelPatchNode*>(parentNode);
#else
		static_cast<ModelPatchNode*>(parentNode);
#endif
	ANKI_ASSERT(modelPatchNode);

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
	const char* name, SceneGraph* scene, SceneNode* parent,
	U32 movableFlags,
	const ModelPatchBase* modelPatch_, U instancesCount)
	:	SceneNode(name, scene, parent),
		Movable(movableFlags, this),
		Renderable(getSceneAllocator()),
		Spatial(&obb, getSceneAllocator()), 
		modelPatch(modelPatch_),
		instances(getSceneAllocator()),
		transforms(getSceneAllocator())
{
	sceneNodeProtected.movable = this;
	sceneNodeProtected.renderable = this;
	sceneNodeProtected.spatial = this;

	Renderable::init(*this);

	// Create the instances as ModelPatchNodeInstance
	if(instancesCount > 1)
	{
		spatialProtected.subSpatials.resize(instancesCount, nullptr);
		instances.resize(instancesCount, nullptr);
		transforms.resize(instancesCount, Transform::getIdentity());

		Vec3 pos = Vec3(0.0);

		for(U i = 0; i < instancesCount; i++)
		{
			ModelPatchNodeInstance* instance = ANKI_NEW(
				ModelPatchNodeInstance, getSceneAllocator(), 
				nullptr, scene, this, Movable::MF_NONE,
				modelPatch);

			instance->setLocalOrigin(pos);
			pos.x() += 2.0;

			spatialProtected.subSpatials[i] = instance;
			instances[i] = instance;
		}
	}
}

//==============================================================================
ModelPatchNode::~ModelPatchNode()
{
	for(ModelPatchNodeInstance* instance : instances)
	{
		ANKI_DELETE(instance, getSceneAllocator());
	}
}

//==============================================================================
const Transform* ModelPatchNode::getRenderableWorldTransforms()
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
void ModelPatchNode::movableUpdate()
{
	Movable::movableUpdate();

	if(instances.size() == 0)
	{
		// NO instancing

		obb = modelPatch->getBoundingShape().getTransformed(
			getWorldTransform());

		spatialMarkForUpdate();
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

	spatialMarkForUpdate();
}

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
ModelNode::ModelNode(
	const char* name, SceneGraph* scene, SceneNode* parent,
	U32 movableFlags,
	const char* modelFname, U instances)
	: 	SceneNode(name, scene, parent),
		Movable(movableFlags, this),
		patches(getSceneAllocator())
{
	sceneNodeProtected.movable = this;

	{
		SceneVector<int> lala(getSceneAllocator());
	}

	model.load(modelFname);

	patches.reserve(model->getModelPatches().size());

	U i = 0;
	for(const ModelPatchBase* patch : model->getModelPatches())
	{
		ModelPatchNode* mpn = ANKI_NEW(ModelPatchNode, getSceneAllocator(),
			nullptr, scene, this,
			Movable::MF_IGNORE_LOCAL_TRANSFORM, patch, instances);

		patches.push_back(mpn);
		++i;
	}
}

//==============================================================================
ModelNode::~ModelNode()
{
	for(ModelPatchNode* patch : patches)
	{
		ANKI_DELETE(patch, getSceneAllocator());
	}
}

//==============================================================================
void ModelNode::setInstanceLocalTransform(U instanceIndex, const Transform& trf)
{
	ANKI_ASSERT(patches.size() > 0);
	ANKI_ASSERT(instanceIndex < patches[0]->instances.size());

	for(ModelPatchNode* patch : patches)
	{
		patch->instances[instanceIndex]->setLocalTransform(trf);
	}
}

} // end namespace anki
