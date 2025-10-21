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

ConstantBuffer<ParticleSimulationConstants> g_consts : register(ANKI_CONCATENATE(b, ANKI_PARTICLE_SIM_CONSTANTS));

RWByteAddressBuffer g_gpuScene : register(ANKI_CONCATENATE(u, ANKI_PARTICLE_SIM_GPU_SCENE));

StructuredBuffer<Mat3x4> g_gpuSceneTransforms : register(ANKI_CONCATENATE(t, ANKI_PARTICLE_SIM_GPU_SCENE_TRANSFORMS));

RWStructuredBuffer<GpuSceneParticleEmitter2> g_gpuSceneParticleEmitters :
	register(ANKI_CONCATENATE(u, ANKI_PARTICLE_SIM_GPU_SCENE_PARTICLE_EMITTERS));

RWStructuredBuffer<ParticleSimulationScratch> g_scratch : register(ANKI_CONCATENATE(u, ANKI_PARTICLE_SIM_SCRATCH));

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
	return genHash(g_randomNumber++);
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
Bool particleCollision(inout Vec3 x, out Vec3 n, F32 acceptablePenetrationDistance = 0.2)
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
	UVec2 texCoord = ndcToUv(v4.xy) * texSize;
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
	texCoord = ndcToUv(v4.xy) * texSize;
	n = unpackNormalFromGBuffer(g_gbufferRt2Tex[texCoord]);

	v4 = mul(g_consts.m_invertedViewProjMat, Vec4(v3.xy, refDepth, 1.0));
	x = v4.xyz / v4.w;

	// Also push it a bit outside the surface
	x += n * 0.1;

	return true;
}

// F Force
// m Mass
// dt Delta time
// v Velocity
// x Particle position
// checkCollision Check collision
// iterationCount The number of interations the simulation will run. Increase it for better accuracy
// e The coefficient of restitution. 0 is inelastic, 1 is bouncy
// mu The friction coefficient. From ~0.2 to 1.0
void simulatePhysics(Vec3 F, F32 m, F32 dt, inout Vec3 v, inout Vec3 x, Bool checkCollision = true, U32 iterationCount = 1, F32 e = 0.5, F32 mu = 0.2)
{
	const Vec3 a = F / m;

	const F32 sdt = dt / F32(iterationCount);
	for(U32 i = 0; i < iterationCount; ++i)
	{
		// Compute the new pos and velocity
		v += a * sdt; // a = dv/dt
		x += v * sdt; // v = dx/dt

		Vec3 n;
		const Bool collides = checkCollision && particleCollision(x, n);
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
		v -= (1.0 + e) * vn * n;

		// Friction
		const Vec3 vt = v - dot(v, n) * n;
		const F32 vtLen = length(vt);
		if(vtLen > 0.0)
		{
			const F32 jn = -(1.0 + e) * m * vn;
			const F32 jtDesired = m * vtLen;
			const F32 jt = min(jtDesired, mu * jn);
			v -= (jt / m) * (vt / vtLen);
		}
	}

	// Add some small damping to avoid jitter
	v *= 0.95;
}
