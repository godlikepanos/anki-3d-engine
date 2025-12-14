// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/Include/ParticleTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>

Texture2D<Vec4> g_depthTex : register(ANKI_CONCATENATE(t, ANKI_PARTICLE_SIM_DEPTH_BUFFER));
Texture2D<Vec4> g_gbufferRt2Tex : register(ANKI_CONCATENATE(t, ANKI_PARTICLE_SIM_NORMAL_BUFFER));

StructuredBuffer<Mat3x4> g_gpuSceneTransforms : register(ANKI_CONCATENATE(t, ANKI_PARTICLE_SIM_GPU_SCENE_TRANSFORMS));

ConstantBuffer<ParticleSimulationConstants> g_consts : register(ANKI_CONCATENATE(b, ANKI_PARTICLE_SIM_CONSTANTS));

RWByteAddressBuffer g_gpuScene : register(ANKI_CONCATENATE(u, ANKI_PARTICLE_SIM_GPU_SCENE));

RWStructuredBuffer<GpuSceneParticleEmitter2> g_gpuSceneParticleEmitters :
	register(ANKI_CONCATENATE(u, ANKI_PARTICLE_SIM_GPU_SCENE_PARTICLE_EMITTERS));

RWStructuredBuffer<ParticleSimulationScratch> g_scratch : register(ANKI_CONCATENATE(u, ANKI_PARTICLE_SIM_SCRATCH));

RWStructuredBuffer<ParticleSimulationCpuFeedback> g_cpuFeedback : register(ANKI_CONCATENATE(u, ANKI_PARTICLE_SIM_CPU_FEEDBACK));

static U32 g_particleIdx;
static U32 g_randomNumber;

void particlesInitGlobals(U32 particleIdx)
{
	g_particleIdx = particleIdx;
	g_randomNumber = g_consts.m_randomNumber + particleIdx;
}

U32 genHash(U32 x)
{
	x ^= x >> 16;
	x *= 0x7feb352d;
	x ^= x >> 15;
	x *= 0x846ca68b;
	x ^= x >> 16;
	return x;
}

U32 getRandomU32()
{
	return genHash(g_particleIdx + g_randomNumber++);
}

F32 getRandomRange(F32 min, F32 max)
{
	const U32 ru = getRandomU32() % 0xFFFFF;
	const F32 r = F32(ru) / F32(0xFFFFF);
	return min + r * (max - min);
}

Vec3 getRandomRange(Vec3 min, Vec3 max)
{
	return Vec3(getRandomRange(min.x, max.x), getRandomRange(min.y, max.y), getRandomRange(min.z, max.z));
}

template<typename T>
T readProp(GpuSceneParticleEmitter2 emitter, ParticleProperty prop)
{
	return g_gpuScene.Load<T>(emitter.m_particleStateSteamOffsets[(U32)prop] + g_particleIdx * sizeof(T));
}

template<typename T>
void writeProp(GpuSceneParticleEmitter2 emitter, ParticleProperty prop, T value)
{
	g_gpuScene.Store<T>(emitter.m_particleStateSteamOffsets[(U32)prop] + g_particleIdx * sizeof(T), value);
}

// Use the depth buffer and the normal buffer to resolve a collision
Bool particleCollision(inout Vec3 x, out Vec3 n, F32 acceptablePenetrationDistance)
{
	n = 0.0;

	Vec4 v4 = mul(g_consts.m_viewProjMat, Vec4(x, 1.0));
	const Vec3 v3 = v4.xyz / v4.w;

	if(any(v3.xy <= -1.0) || any(v3.xy >= 1.0))
	{
		return false;
	}

	Vec2 texSize;
	g_depthTex.GetDimensions(texSize.x, texSize.y);
	UVec2 texCoord = ndcToUv(v3.xy) * texSize;
	const F32 refDepth = g_depthTex[texCoord].r;
	const F32 particleDepth = v3.z;
	if(particleDepth < refDepth)
	{
		return false;
	}

	const F32 refViewZ = g_consts.m_unprojectionParams.z / (g_consts.m_unprojectionParams.w + refDepth);
	const F32 particleViewZ = g_consts.m_unprojectionParams.z / (g_consts.m_unprojectionParams.w + particleDepth);
	if(abs(particleViewZ - refViewZ) > acceptablePenetrationDistance)
	{
		// Depth buffer is not reliable, assume no collision
		return false;
	}

	// Collides, change the position

	g_gbufferRt2Tex.GetDimensions(texSize.x, texSize.y);
	texCoord = ndcToUv(v3.xy) * texSize;
	n = unpackNormalFromGBuffer(g_gbufferRt2Tex[texCoord]);

	v4 = mul(g_consts.m_invertedViewProjMat, Vec4(v3.xy, refDepth, 1.0));
	x = v4.xyz / v4.w;

	// Also push it a bit outside the surface
	x += n * 0.01;

	return true;
}

struct SimulationArgs
{
	Bool m_checkCollision; // Check collision using the depth buffer
	F32 m_penetrationDistance; // Since collision is checked against the depth buffer add a threshold to avoid falce positives

	U32 m_iterationCount; // The number of interations the simulation will run. Increase it for better accuracy
	F32 m_e; // The coefficient of restitution. 0 is inelastic, 1 is bouncy
	F32 m_mu; // The friction coefficient. From ~0.2 to 1.0

	F32 m_velocityDamping; // Decreases the velocity a bit. Set it to 1 to disable damping

	void init()
	{
		m_checkCollision = true;
		m_penetrationDistance = 0.5;
		m_iterationCount = 1;
		m_e = 0.5;
		m_mu = 0.2;
		m_velocityDamping = 1.0;
	}
};

// F Force
// m Mass
// dt Delta time
// v Velocity
// x Particle position
void simulatePhysics(Vec3 F, F32 m, F32 dt, inout Vec3 v, inout Vec3 x, SimulationArgs args)
{
	const Vec3 a = F / m;

	const F32 sdt = dt / F32(args.m_iterationCount);
	for(U32 i = 0; i < args.m_iterationCount; ++i)
	{
		// Compute the new pos and velocity
		v += a * sdt; // a = dv/dt
		x += v * sdt; // v = dx/dt

		Vec3 n;
		const Bool collides = args.m_checkCollision && particleCollision(x, n, args.m_penetrationDistance);
		if(!collides)
		{
			continue;
		}

		const F32 vn = dot(v, n);
		if(vn >= 0.0)
		{
			continue;
		}

		// Restitution
		v -= (1.0 + args.m_e) * vn * n;

		// Friction
		const Vec3 vt = v - dot(v, n) * n;
		const F32 vtLen = length(vt);
		if(vtLen > 0.0)
		{
			const F32 jn = -(1.0 + args.m_e) * m * vn;
			const F32 jtDesired = m * vtLen;
			const F32 jt = min(jtDesired, args.m_mu * jn);
			v -= (jt / m) * (vt / vtLen);
		}
	}

	// Add some small damping to avoid jitter
	v *= args.m_velocityDamping;
}

void appendAlive(GpuSceneParticleEmitter2 emitter, Vec3 particlePos, F32 particleScale)
{
	// Add the alive particle index to the array
	U32 count;
	InterlockedAdd(g_scratch[0].m_aliveParticleCount, 1, count);
	BAB_STORE(g_gpuScene, U32, emitter.m_aliveParticleIndicesOffset + count * sizeof(U32), g_particleIdx);

	// Update the AABB
	const F32 toCentimeters = 100.0;
	const IVec3 quatizedPosMin = floor((particlePos + emitter.m_particleAabbMin * particleScale) * toCentimeters);
	const IVec3 quatizedPosMax = ceil((particlePos + emitter.m_particleAabbMax * particleScale) * toCentimeters);
	[unroll] for(U32 i = 0; i < 3; ++i)
	{
		InterlockedMin(g_scratch[0].m_aabbMin[i], quatizedPosMin[i]);
		InterlockedMax(g_scratch[0].m_aabbMax[i], quatizedPosMax[i]);
	}
}

template<typename TInterface>
void particleMain(U32 svDispatchThreadId, U32 svGroupIndex, TInterface iface)
{
	particlesInitGlobals(svDispatchThreadId.x);

	GpuSceneParticleEmitter2 emitter = SBUFF(g_gpuSceneParticleEmitters, g_consts.m_gpuSceneParticleEmitterIndex);
	iface.initAnKiParticleEmitterProperties(emitter);
	const Mat3x4 emitterTrf = SBUFF(g_gpuSceneTransforms, emitter.m_worldTransformsIndex);

	const Bool reinit = emitter.m_reinitializeOnNextUpdate;
	const Bool canEmitThisFrame = emitter.m_timeLeftForNextEmission - g_consts.m_dt <= 0.0;

	if(g_particleIdx < emitter.m_particleCount)
	{
		// Decide what to do
		Bool init = false;
		Bool makeAlive = false;
		Bool simulate = false;
		F32 lifeFactor = 1.0;

		if(reinit)
		{
			U32 emittedParticleCount;
			InterlockedAdd(g_scratch[0].m_emittedParticleCount, 1, emittedParticleCount);

			init = true;
			makeAlive = emittedParticleCount < emitter.m_particlesPerEmission;
		}
		else
		{
			lifeFactor = readProp<F32>(emitter, ParticleProperty::kLifeFactor);
			const Bool alive = lifeFactor < 1.0;

			if(alive)
			{
				simulate = true;
			}
			else if(canEmitThisFrame)
			{
				U32 emittedParticleCount;
				InterlockedAdd(g_scratch[0].m_emittedParticleCount, 1, emittedParticleCount);

				init = emittedParticleCount < emitter.m_particlesPerEmission;
				makeAlive = true;
			}
		}

		// Do the actual work
		if(simulate)
		{
			Vec3 particlePosition;
			F32 particleScale;
			iface.simulateParticle(emitter, lifeFactor, particlePosition, particleScale);
			appendAlive(emitter, particlePosition, particleScale);
		}
		else if(init)
		{
			Vec3 particlePosition;
			F32 particleScale;
			iface.initializeParticle(emitter, emitterTrf, makeAlive, particlePosition, particleScale);

			if(makeAlive)
			{
				appendAlive(emitter, particlePosition, particleScale);
			}
		}
	}

	// Check if it's the last threadgroup running
	if(svGroupIndex == 0)
	{
		U32 threadgroupIdx;
		InterlockedAdd(g_scratch[0].m_threadgroupCount, 1, threadgroupIdx);
		const U32 threadgroupCount = (emitter.m_particleCount + ANKI_WAVE_SIZE - 1) / ANKI_WAVE_SIZE;
		const Bool lastThreadExecuting = (threadgroupIdx + 1 == threadgroupCount);

		if(lastThreadExecuting)
		{
			// Inform about the bounding volume
			const F32 toMeters = 1.0 / 100.0;
			ParticleSimulationCpuFeedback feedback = (ParticleSimulationCpuFeedback)0;
			feedback.m_aabbMin = g_scratch[0].m_aabbMin * toMeters;
			feedback.m_aabbMax = g_scratch[0].m_aabbMax * toMeters;
			feedback.m_uuid = emitter.m_uuid;
			g_cpuFeedback[0] = feedback;

			// Update the GPU scene emitter
			if(canEmitThisFrame)
			{
				SBUFF(g_gpuSceneParticleEmitters, g_consts.m_gpuSceneParticleEmitterIndex).m_timeLeftForNextEmission = emitter.m_emissionPeriod;
			}
			else
			{
				SBUFF(g_gpuSceneParticleEmitters, g_consts.m_gpuSceneParticleEmitterIndex).m_timeLeftForNextEmission -= g_consts.m_dt;
			}

			if(reinit)
			{
				SBUFF(g_gpuSceneParticleEmitters, g_consts.m_gpuSceneParticleEmitterIndex).m_reinitializeOnNextUpdate = 0;
			}

			SBUFF(g_gpuSceneParticleEmitters, g_consts.m_gpuSceneParticleEmitterIndex).m_aliveParticleCount = g_scratch[0].m_aliveParticleCount;

			// Reset the scratch struct for next frame
			g_scratch[0].m_aabbMin = kMaxI32;
			g_scratch[0].m_aabbMax = kMinI32;
			g_scratch[0].m_threadgroupCount = 0;
			g_scratch[0].m_emittedParticleCount = 0;
			g_scratch[0].m_aliveParticleCount = 0;
		}
	}
}
