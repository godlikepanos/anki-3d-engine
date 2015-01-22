// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/PlayerNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/PlayerControllerComponent.h"
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
		updated = false;

		PlayerControllerComponent& playerc = 
			node.getComponent<PlayerControllerComponent>();

		// Process physics move
		if(playerc.getTimestamp() == getGlobTimestamp())
		{
			MoveComponent& move = node.getComponent<MoveComponent>();
			move.setLocalTransform(playerc.getTransform());
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// PlayerNodeFeedbackComponent2                                                =
//==============================================================================

/// Feedback component.
class PlayerNodeFeedbackComponent2 final: public SceneComponent
{
public:
	PlayerNodeFeedbackComponent2(SceneNode* node)
	:	SceneComponent(SceneComponent::Type::NONE, node)
	{}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;

		PlayerControllerComponent& playerc = 
			node.getComponent<PlayerControllerComponent>();
		MoveComponent& move = node.getComponent<MoveComponent>();

		// Process input
		const Input& in = node.getSceneGraph().getInput();
		Vec4 moveVec(0.0);

		if(in.getKey(KeyCode::W))
		{
			moveVec.z() += 1.0;
		}

		if(in.getKey(KeyCode::A))
		{
			moveVec.x() -= 1.0;
		}

		if(in.getKey(KeyCode::S))
		{
			moveVec.z() -= 1.0;
		}

		if(in.getKey(KeyCode::D))
		{
			moveVec.x() += 1.0;
		}

		const Transform& trf = playerc.getTransform();
		Vec4 dir = trf.getRotation().getColumn(2).xyz0();

		if(in.getKey(KeyCode::LEFT))
		{
			Mat3x4 rot = trf.getRotation();
			rot.rotateYAxis(toRad(10.0));

			dir = rot.getColumn(2).xyz0();
		}

		if(in.getKey(KeyCode::RIGHT))
		{
			Mat3x4 rot = trf.getRotation();
			rot.rotateYAxis(toRad(-10.0));

			dir = rot.getColumn(2).xyz0();
		}

		const F32 speed = 2.0;
		playerc.setVelocity(
			moveVec.z() * speed, 
			moveVec.x() * speed,
			0.0, 
			dir);

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

	// Player controller component
	comp = getSceneAllocator().newInstance<PlayerControllerComponent>(
		this, m_player);
	if(comp == nullptr)
	{
		return ErrorCode::OUT_OF_MEMORY; 
	}

	err = addComponent(comp, true);
	if(err)
	{
		return err;
	}

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

	// Feedback component #2
	comp = getSceneAllocator().newInstance<PlayerNodeFeedbackComponent2>(this);
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

