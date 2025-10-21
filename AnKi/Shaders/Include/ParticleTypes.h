// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

/// Some data that are used temporarely by a simulation.
struct ParticleSimulationScratch
{
	IVec3 m_aabbMin; // U32 because of atomics. In cm
	U32 m_threadgroupCount;

	IVec3 m_aabbMax; // U32 because of atomics. In cm
	U32 m_emittedParticleCount;
};

/// Constants used in the simulation.
struct ParticleSimulationConstants
{
	Mat4 m_viewProjMat;

	Mat4 m_invertedViewProjMat;

	Vec4 m_unprojectionParams;

	U32 m_randomNumber;
	F32 m_dt;
	U32 m_gpuSceneParticleEmitterIndex;
	U32 m_padding0;
};

/// The various properties of a GPU particle.
enum class ParticleProperty
{
	kVelocity, // Vec3
	kLife, // F32
	kDeathTime, // F32
	kPosition, // Vec3
	kScale, // Vec3
	kRotation, // Quat: 4xF32
	kPreviousPosition, // Vec3
	kPreviousRotation, // Quat: 4xF32
	kPreviousScale, // Vec3
	kMass, // F32
	kUserDefined0, // 4x32bit
	kUserDefined1, // 4x32bit
	kUserDefined2, // 4x32bit

	kCount,
	kFirst = 0
};

// SRV
#define ANKI_PARTICLE_SIM_DEPTH_BUFFER 0
#define ANKI_PARTICLE_SIM_NORMAL_BUFFER 1
#define ANKI_PARTICLE_SIM_GPU_SCENE_TRANSFORMS 2

// UAV
#define ANKI_PARTICLE_SIM_SCRATCH 0
#define ANKI_PARTICLE_SIM_GPU_SCENE 1
#define ANKI_PARTICLE_SIM_GPU_SCENE_PARTICLE_EMITTERS 2

// CBV
#define ANKI_PARTICLE_SIM_CONSTANTS 0

ANKI_END_NAMESPACE
