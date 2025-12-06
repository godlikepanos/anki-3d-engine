// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common stuff for GBuffer, Forward shading and the rest of shaders that appear in materials.

#pragma once

#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/MeshTypes.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>
#include <AnKi/Shaders/VisibilityAndCollisionFunctions.hlsl>
#include <AnKi/Shaders/PackFunctions.hlsl>

#define _ANKI_REG(type, binding) type##binding
#define ANKI_REG(type, binding) _ANKI_REG(type, binding)

SamplerState g_globalSampler : register(ANKI_REG(s, ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER));
ConstantBuffer<MaterialGlobalConstants> g_globalConstants : register(ANKI_REG(b, ANKI_MATERIAL_REGISTER_GLOBAL_CONSTANTS));
ByteAddressBuffer g_gpuScene : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_GPU_SCENE));

// Unified geom:
ByteAddressBuffer g_unifiedGeom : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_UNIFIED_GEOMETRY));
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) Buffer<shaderType> g_unifiedGeom_##fmt : register(ANKI_REG(t, reg));
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

StructuredBuffer<MeshletBoundingVolume> g_meshletBoundingVolumes : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_MESHLET_BOUNDING_VOLUMES));
StructuredBuffer<MeshletGeometryDescriptor> g_meshletGeometryDescriptors : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_MESHLET_GEOMETRY_DESCRIPTORS));
StructuredBuffer<GpuSceneMeshletInstance> g_meshletInstances : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_MESHLET_INSTANCES));
StructuredBuffer<GpuSceneRenderable> g_renderables : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_RENDERABLES));
StructuredBuffer<GpuSceneMeshLod> g_meshLods : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_MESH_LODS));
StructuredBuffer<GpuSceneParticleEmitter2> g_particleEmitters2 : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_PARTICLE_EMITTERS2));
StructuredBuffer<Mat3x4> g_transforms : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_TRANSFORMS));
SamplerState g_nearestClampSampler : register(ANKI_REG(s, ANKI_MATERIAL_REGISTER_NEAREST_CLAMP_SAMPLER));
StructuredBuffer<U32> g_firstMeshlet : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_FIRST_MESHLET));
StructuredBuffer<GpuScenePerDraw> g_perDraw : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_PER_DRAW));

// One for each bucket. Points to g_perDraw
StructuredBuffer<U32> g_firstPerDraw : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_PER_DRAW_OFFSET));

#if ANKI_MESH_SHADER || ANKI_VERTEX_SHADER
struct Consts
{
	U32 m_bucketIndex;
	U32 m_padding1;
	U32 m_padding2;
	U32 m_padding3;
};
ANKI_FAST_CONSTANTS(Consts, g_consts)
#endif

// FW shading specific
#if defined(FORWARD_SHADING)
#	include <AnKi/Shaders/ClusteredShadingFunctions.hlsl>

SamplerState g_linearAnyClampSampler : register(ANKI_REG(s, ANKI_MATERIAL_REGISTER_LINEAR_CLAMP_SAMPLER));
Texture2D g_gbufferDepthTex : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_SCENE_DEPTH));
Texture3D<Vec4> g_lightVol : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_LIGHT_VOLUME));
SamplerComparisonState g_shadowSampler : register(ANKI_REG(s, ANKI_MATERIAL_REGISTER_SHADOW_SAMPLER));

ConstantBuffer<GlobalRendererConstants> g_globalRendererConstants : register(ANKI_REG(b, ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_CONSTANTS));
StructuredBuffer<Cluster> g_clusters : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_CLUSTERS));
StructuredBuffer<PointLight> g_pointLights : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_POINT_LIGHTS));
StructuredBuffer<SpotLight> g_spotLights : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_SPOT_LIGHTS));
Texture2D<Vec4> g_shadowAtlasTex : register(ANKI_REG(t, ANKI_MATERIAL_REGISTER_SHADOW_ATLAS));
#endif

#undef ANKI_REG

#if ANKI_VERTEX_SHADER
GpuScenePerDraw getGpuScenePerDraw()
{
	return g_perDraw[gl_DrawID + g_firstPerDraw[g_consts.m_bucketIndex]];
}
#endif

/// Used in vert shading.
UnpackedMeshVertex loadVertex(GpuSceneMeshLod mlod, U32 svVertexId, Bool bones)
{
	UnpackedMeshVertex v;
	v.m_position = g_unifiedGeom_R16G16B16A16_Unorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kPosition] + svVertexId];
	v.m_position = v.m_position * mlod.m_positionScale + mlod.m_positionTranslation;

	v.m_normal = g_unifiedGeom_R8G8B8A8_Snorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kNormal] + svVertexId].xyz;
	v.m_uv = g_unifiedGeom_R32G32_Sfloat[mlod.m_vertexOffsets[(U32)VertexStreamId::kUv] + svVertexId];

	if(bones)
	{
		v.m_boneIndices = g_unifiedGeom_R8G8B8A8_Uint[mlod.m_vertexOffsets[(U32)VertexStreamId::kBoneIds] + svVertexId];
		v.m_boneWeights = g_unifiedGeom_R8G8B8A8_Snorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kBoneWeights] + svVertexId];
	}

	return v;
}

/// Used in mesh shading.
UnpackedMeshVertex loadVertexLocalIndex(MeshletGeometryDescriptor meshlet, U32 localIdx, Bool bones)
{
	UnpackedMeshVertex v;
	v.m_position = g_unifiedGeom_R16G16B16A16_Unorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kPosition] + localIdx];
	v.m_position = v.m_position * meshlet.m_positionScale + meshlet.m_positionTranslation;

	v.m_normal = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kNormal] + localIdx].xyz;
	v.m_uv = g_unifiedGeom_R32G32_Sfloat[meshlet.m_vertexOffsets[(U32)VertexStreamId::kUv] + localIdx];

	if(bones)
	{
		v.m_boneIndices = g_unifiedGeom_R8G8B8A8_Uint[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneIds] + localIdx];
		v.m_boneWeights = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneWeights] + localIdx];
	}

	return v;
}

/// Used in SW meshlet rendering.
UnpackedMeshVertex loadVertex(MeshletGeometryDescriptor meshlet, U32 svVertexId, Bool bones)
{
	// Indices are stored in R8G8B8A8_Uint per primitive. Last component is not used.
	const U32 primitiveId = svVertexId / 3u;
	const U32 primitiveIndices = g_unifiedGeom.Load<U32>((meshlet.m_firstPrimitive + primitiveId) * sizeof(U32));
	const U32 vertOfPrimitive = svVertexId % 3u;
	const U32 localIdx = (primitiveIndices >> (vertOfPrimitive * 8u)) & 0xFF;

	return loadVertexLocalIndex(meshlet, localIdx, bones);
}

Bool cullBackfaceMeshlet(MeshletBoundingVolume meshlet, Mat3x4 worldTransform, Vec3 cameraWorldPos)
{
	const Vec4 coneDirAndAng = unpackSnorm4x8<F32>(meshlet.m_coneDirection_R8G8B8_Snorm_cosHalfAngle_R8_Snorm);
	return cullBackfaceMeshlet(coneDirAndAng.xyz, coneDirAndAng.w, meshlet.m_coneApex, worldTransform, cameraWorldPos);
}
