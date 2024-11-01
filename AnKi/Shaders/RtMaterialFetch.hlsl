// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/Common.hlsl>

struct [raypayload] RtMaterialFetchRayPayload
{
	Vec3 m_diffuseColor : write(closesthit, miss): read(caller);
	Vec3 m_worldNormal : write(closesthit, miss): read(caller);
	Vec3 m_emission : write(closesthit, miss): read(caller);
	F32 m_rayT : write(closesthit, miss): read(caller);
};

// Have a common resouce interface for all shaders
#if ANKI_RAY_GEN_SHADER
#	define SPACE space2

ConstantBuffer<GlobalRendererConstants> g_globalRendererConstants : register(b0, SPACE);

RaytracingAccelerationStructure g_tlas : register(t0, SPACE);
Texture2D<Vec4> g_depthTex : register(t1, SPACE);
Texture2D<Vec4> g_gbufferRt1 : register(t2, SPACE);
Texture2D<Vec4> g_gbufferRt2 : register(t3, SPACE);
Texture2D<Vec4> g_envMap : register(t4, SPACE);

RWTexture2D<Vec4> g_colorAndPdfTex : register(u0, SPACE);
RWTexture2D<Vec4> g_hitPosAndDepthTex : register(u1, SPACE);

SamplerState g_linearClampAnySampler : register(s0, SPACE);
#endif
