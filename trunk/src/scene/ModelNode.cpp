#include "anki/scene/ModelNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/InstanceNode.h"
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
	const U32* subMeshIndicesArray, U subMeshIndicesCount,
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

	dc.instancesCount = spatialsCount;

	modelPatch->getRenderingDataSub(key, vao, prog, 
		subMeshIndicesArray, subMeshIndicesCount, 
		dc.countArray, dc.offsetArray, dc.drawCount);
}

//==============================================================================
const Transform* ModelPatchNode::getRenderWorldTransforms()
{
	SceneNode* parent = staticCast<SceneNode*>(getParent());
	ANKI_ASSERT(parent);
	MoveComponent& move = parent->getComponent<MoveComponent>();

	if(1) // XXX
	{
		// NO instancing
		return &move.getWorldTransform();
	}
	else
	{
		// Instancing

		// XXX
		ANKI_ASSERT(0 && "todo");
	}
}

//==============================================================================
void ModelPatchNode::frameUpdate(F32, F32, SceneNode::UpdateType uptype)
{
	if(uptype != SceneNode::ASYNC_UPDATE)
	{
		return;
	}

	SceneNode* parent = staticCast<SceneNode*>(getParent());
	ANKI_ASSERT(parent);

	// Count the instances of the parent
	U instancesCount = 0;
	parent->visitThisAndChildren([&](SceneObject& so)
	{
		SceneNode* sn = staticCast<SceneNode*>(&so);
		
		if(sn->tryGetComponent<InstanceComponent>())
		{
			++instancesCount;
		}
	});

	if(instancesCount == 0)
	{
		// No instances
		const MoveComponent& parentMove = parent->getComponent<MoveComponent>();

		if(parentMove.getTimestamp() == getGlobTimestamp())
		{
			obb = modelPatch->getBoundingShape().getTransformed(
				parentMove.getWorldTransform());
		}
	}
	else
	{
		// XXX
		ANKI_ASSERT(0 && "todo");
	}
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
		transforms(getSceneAllocator())
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

} // end namespace anki
