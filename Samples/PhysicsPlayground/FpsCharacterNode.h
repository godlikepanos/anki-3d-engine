// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/AnKi.h>
#include <Samples/PhysicsPlayground/GrenadeNode.h>

using namespace anki;

ANKI_CVAR(NumericCVar<F32>, Game, Fov, 90.0f, 60.0f, 110.0f, "Field of view")
ANKI_CVAR(NumericCVar<F32>, Game, MouseLookPower, 5.0f, 1.0f, 100.0f, "Mouselook")

class FpsCharacter final : public SceneNode
{
	ANKI_REGISTER_SCENE_NODE_CLASS(FpsCharacter)

public:
	F32 m_walkingSpeed = 8.5f;
	F32 m_jumpSpeed = 8.0f;
	Bool m_crouching = false;

	U8 m_shotgunBulletCount = 8;
	F32 m_shotgunSpreadAngle = 6.0_degrees;
	F32 m_shotgunMaxLength = 100.0f;
	F32 m_shotgunForce = 80.0f;
	Vec3 m_shotgunRestingPosition = Vec3(0.1f, -0.18f, -0.19f);
	Euler m_shotgunRestingRotation = Euler(0.0f, kPi, 0.0f);

	SceneNode* m_cameraNode = nullptr;
	SceneNode* m_shotgunNode = nullptr;

	FpsCharacter(const SceneNodeInitInfo& inf);

private:
	void fireShotgun();
	void fireGrenade();

	void update(SceneNodeUpdateInfo& info) override;

	Error serialize(SceneSerializer& serializer) override;
};
