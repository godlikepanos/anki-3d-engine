// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/include/Common.h>

ANKI_BEGIN_NAMESPACE

// Particle emitter properties
struct GpuParticleEmitterProperties
{
	Vec3 m_minGravity;
	F32 m_minMass;

	Vec3 m_maxGravity;
	F32 m_maxMass;

	Vec3 m_minForce;
	F32 m_minLife;

	Vec3 m_maxForce;
	F32 m_maxLife;

	Vec3 m_minStartingPosition;
	F32 m_padding0;

	Vec3 m_maxStartingPosition;
	U32 m_particleCount;
};

// GPU particle state
struct GpuParticle
{
	Vec3 m_oldWorldPosition;
	F32 m_mass;

	Vec3 m_newWorldPosition;
	F32 m_life;

	Vec3 m_acceleration;
	F32 m_startingLife; // The original max life

	Vec3 m_velocity;
	F32 m_padding1;
};

struct GpuParticleSimulationState
{
	Mat4 m_viewProjMat;

	Vec4 m_unprojectionParams;

	Vec2 m_padding0;
	U32 m_randomIndex;
	F32 m_dt;

	Vec3 m_emitterPosition;
	F32 m_padding1;

#if defined(__cplusplus)
	Mat3x4 m_emitterRotation;
#else
	Mat3 m_emitterRotation;
#endif

#if defined(__cplusplus)
	Mat3x4 m_invViewRotation;
#else
	Mat3 m_invViewRotation;
#endif
};

ANKI_END_NAMESPACE
