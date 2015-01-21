// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/PlayerNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/physics/PhysicsPlayerController.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/input/Input.h"

namespace anki {

//==============================================================================
// PlayerNodeFeedbackComponent                                                 =
//==============================================================================

/// Feedback component.
class PlayerNodeFeedbackComponent final: public SceneComponent
{
public:
	PlayerNodeFeedbackComponent(SceneNode* node)
	:	SceneComponent(SceneComponent::Type::NONE, node)
	{}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		PlayerNode& pnode = static_cast<PlayerNode&>(node);

		// Process physics move
		const Transform& trf = pnode.m_player->getTransform(updated);
		if(updated)
		{
			MoveComponent& move = node.getComponent<MoveComponent>();
			move.setLocalTransform(trf);
		}

		// Process input
		const Input& in = node.getSceneGraph().getInput();
		Vec4 moveVec(0.0);

		if(in.getKey(KeyCode::W))
		{
			moveVec.z() += -1.0;
		}

		if(in.getKey(KeyCode::A))
		{
			moveVec.x() += -1.0;
		}

		if(in.getKey(KeyCode::S))
		{
			moveVec.z() += 1.0;
		}

		if(in.getKey(KeyCode::D))
		{
			moveVec.x() += 1.0;
		}

		//if(moveVec != Vec4(0.0))
		{
			const F32 speed = 2.0;
			pnode.m_player->setVelocity(
				moveVec.x() * speed, 
				moveVec.z() * speed,
				0.0, 
				Vec4(0.0, 0.0, -1.0, 0.0));
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// PlayerNode                                                                  =
//==============================================================================

//==============================================================================
PlayerNode::PlayerNode(SceneGraph* scene)
:	SceneNode(scene)
{}

//==============================================================================
PlayerNode::~PlayerNode()
{
	if(m_player)
	{
		m_player->setMarkedForDeletion();	
	}
}

//==============================================================================
Error PlayerNode::create(const CString& name)
{
	Error err = SceneNode::create(name);
	if(err)
	{
		return err;
	}

	// Create physics object
	PhysicsPlayerController::Initializer init;
	m_player = getSceneGraph()._getPhysicsWorld().newPlayerController(init);

	SceneComponent* comp;

	// Feedback component
	comp = getSceneAllocator().newInstance<PlayerNodeFeedbackComponent>(this);
	if(comp == nullptr)
	{
		return ErrorCode::OUT_OF_MEMORY; 
	}

	err = addComponent(comp, true);
	if(err)
	{
		return err;
	}

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	if(comp == nullptr)
	{
		return ErrorCode::OUT_OF_MEMORY; 
	}

	err = addComponent(comp, true);
	if(err)
	{
		return err;
	}

	return err;
}

} // end namespace anki

