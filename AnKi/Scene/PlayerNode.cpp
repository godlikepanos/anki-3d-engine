// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/PlayerNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Physics/PhysicsPlayerController.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Input/Input.h>

namespace anki {

/// Feedback component.
class PlayerNode::FeedbackComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(PlayerNode::FeedbackComponent)

public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), false)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		PlayerControllerComponent& playerc = info.m_node->getFirstComponentOfType<PlayerControllerComponent>();
		const Input& in = info.m_node->getSceneGraph().getInput();
		const F32 ang = toRad(7.0f);

		F32 y = in.getMousePosition().y();
		F32 x = in.getMousePosition().x();
		if(playerc.getTimestamp() == info.m_node->getGlobalTimestamp() || y != 0.0 || x != 0.0)
		{
			MoveComponent& move = info.m_node->getFirstComponentOfType<MoveComponent>();

			// Set origin
			Vec4 origin = playerc.getWorldTransform().getOrigin();
			origin.y() += 1.9f;

			// Set rotation
			Mat3x4 rot(Vec3(0.0f), Euler(ang * y * 11.25f, ang * x * -20.0f, 0.0f));

			rot = move.getLocalRotation().combineTransformations(rot);

			Vec3 newz = rot.getColumn(2).getNormalized();
			Vec3 newx = Vec3(0.0, 1.0, 0.0).cross(newz);
			Vec3 newy = newz.cross(newx);
			rot.setColumns(newx, newy, newz, Vec3(0.0));
			rot.reorthogonalize();

			// Update move
			move.setLocalTransform(Transform(origin, rot, 1.0));
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(PlayerNode::FeedbackComponent)

/// Feedback component.
class PlayerNode::FeedbackComponent2 final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(PlayerNode::FeedbackComponent2)

public:
	FeedbackComponent2(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), false // This shouldn't behave as a feedback component
		)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		PlayerControllerComponent& playerc = info.m_node->getFirstComponentOfType<PlayerControllerComponent>();
		MoveComponent& move = info.m_node->getFirstComponentOfType<MoveComponent>();
		const Input& in = info.m_node->getSceneGraph().getInput();

		const F32 speed = 0.5;

		Vec4 moveVec(0.0);
		if(in.getKey(KeyCode::kW))
		{
			moveVec.z() += 1.0f;
		}

		if(in.getKey(KeyCode::kA))
		{
			moveVec.x() -= 1.0f;
		}

		if(in.getKey(KeyCode::kS))
		{
			moveVec.z() -= 1.0f;
		}

		if(in.getKey(KeyCode::kD))
		{
			moveVec.x() += 1.0f;
		}

		Vec4 dir = -move.getLocalRotation().getZAxis().xyz0();
		dir.y() = 0.0f;
		dir.normalize();

		playerc.setVelocity(moveVec.z() * speed, moveVec.x() * speed, 0.0, dir);

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(PlayerNode::FeedbackComponent2)

PlayerNode::PlayerNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<PlayerControllerComponent>();
	newComponent<FeedbackComponent>();
	newComponent<MoveComponent>();
	newComponent<FeedbackComponent2>();
}

PlayerNode::~PlayerNode()
{
}

} // end namespace anki
