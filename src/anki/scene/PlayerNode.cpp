// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/PlayerNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/PlayerControllerComponent.h>
#include <anki/physics/PhysicsPlayerController.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/input/Input.h>

namespace anki
{

/// Feedback component.
class PlayerNodeFeedbackComponent final : public SceneComponent
{
public:
	PlayerNodeFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;

		PlayerControllerComponent& playerc = node.getComponent<PlayerControllerComponent>();
		const Input& in = node.getSceneGraph().getInput();
		const F32 ang = toRad(7.0);

		F32 y = in.getMousePosition().y();
		F32 x = in.getMousePosition().x();
		if(playerc.getTimestamp() == node.getGlobalTimestamp() || y != 0.0 || x != 0.0)
		{
			MoveComponent& move = node.getComponent<MoveComponent>();

			// Set origin
			Vec4 origin = playerc.getTransform().getOrigin();
			origin.y() += 1.9;

			// Set rotation
			Mat3x4 rot(Euler(ang * y * 11.25, ang * x * -20.0, 0.0));

			rot = move.getLocalRotation().combineTransformations(rot);

			Vec3 newz = rot.getColumn(2).getNormalized();
			Vec3 newx = Vec3(0.0, 1.0, 0.0).cross(newz);
			Vec3 newy = newz.cross(newx);
			rot.setColumns(newx, newy, newz, Vec3(0.0));
			rot.reorthogonalize();

			// Update move
			move.setLocalTransform(Transform(origin, rot, 1.0));
		}

		return ErrorCode::NONE;
	}
};

/// Feedback component.
class PlayerNodeFeedbackComponent2 final : public SceneComponent
{
public:
	PlayerNodeFeedbackComponent2(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;

		PlayerControllerComponent& playerc = node.getComponent<PlayerControllerComponent>();
		MoveComponent& move = node.getComponent<MoveComponent>();
		const Input& in = node.getSceneGraph().getInput();

		const F32 speed = 3.5;

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

		Vec4 dir = -move.getLocalRotation().getZAxis().xyz0();
		dir.y() = 0.0;
		dir.normalize();

		playerc.setVelocity(moveVec.z() * speed, moveVec.x() * speed, 0.0, dir);

		return ErrorCode::NONE;
	}
};

PlayerNode::PlayerNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

PlayerNode::~PlayerNode()
{
}

Error PlayerNode::init(const Vec4& position)
{
	// Create physics object
	PhysicsPlayerControllerInitInfo init;
	init.m_position = position;
	m_player = getSceneGraph().getPhysicsWorld().newInstance<PhysicsPlayerController>(init);

	SceneComponent* comp;

	// Player controller component
	comp = getSceneAllocator().newInstance<PlayerControllerComponent>(this, m_player);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().newInstance<PlayerNodeFeedbackComponent>(this);
	addComponent(comp, true);

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Feedback component #2
	comp = getSceneAllocator().newInstance<PlayerNodeFeedbackComponent2>(this);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

} // end namespace anki
