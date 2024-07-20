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

SamplerState g_globalSampler : register(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER);
ConstantBuffer<MaterialGlobalUniforms> g_globalUniforms : register(ANKI_MATERIAL_REGISTER_GLOBAL_UNIFORMS);
ByteAddressBuffer g_gpuScene : register(ANKI_MATERIAL_REGISTER_GPU_SCENE);

// Unified geom:
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) Buffer<shaderType> g_unifiedGeom_##fmt : register(reg);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

StructuredBuffer<MeshletBoundingVolume> g_meshletBoundingVolumes : register(ANKI_MATERIAL_REGISTER_MESHLET_BOUNDING_VOLUMES);
StructuredBuffer<MeshletGeometryDescriptor> g_meshletGeometryDescriptors : register(ANKI_MATERIAL_REGISTER_MESHLET_GEOMETRY_DESCRIPTORS);
StructuredBuffer<GpuSceneMeshletGroupInstance> g_meshletGroups : register(ANKI_MATERIAL_REGISTER_MESHLET_GROUPS);
StructuredBuffer<GpuSceneRenderable> g_renderables : register(ANKI_MATERIAL_REGISTER_RENDERABLES);
StructuredBuffer<GpuSceneMeshLod> g_meshLods : register(ANKI_MATERIAL_REGISTER_MESH_LODS);
StructuredBuffer<Mat3x4> g_transforms : register(ANKI_MATERIAL_REGISTER_TRANSFORMS);
Texture2D<Vec4> g_hzbTexture : register(ANKI_MATERIAL_REGISTER_HZB_TEXTURE);
SamplerState g_nearestClampSampler : register(ANKI_MATERIAL_REGISTER_NEAREST_CLAMP_SAMPLER);
StructuredBuffer<GpuSceneParticleEmitter> g_particleEmitters : register(ANKI_MATERIAL_REGISTER_PARTICLE_EMITTERS);

// FW shading specific
#if defined(FORWARD_SHADING)
#	include <AnKi/Shaders/ClusteredShadingFunctions.hlsl>

SamplerState g_linearAnyClampSampler : register(ANKI_MATERIAL_REGISTER_LINEAR_CLAMP_SAMPLER);
Texture2D g_gbufferDepthTex : register(ANKI_MATERIAL_REGISTER_SCENE_DEPTH);
Texture3D<RVec4> g_lightVol : register(ANKI_MATERIAL_REGISTER_LIGHT_VOLUME);
SamplerComparisonState g_shadowSampler : register(ANKI_MATERIAL_REGISTER_SHADOW_SAMPLER);

ConstantBuffer<GlobalRendererUniforms> g_globalRendererUniforms : register(ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_UNIFORMS);
StructuredBuffer<Cluster> g_clusters : register(ANKI_MATERIAL_REGISTER_CLUSTERS);
StructuredBuffer<PointLight> g_pointLights : register(ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_POINT_LIGHTS);
StructuredBuffer<SpotLight> g_spotLights : register(ANKI_MATERIAL_REGISTER_CLUSTER_SHADING_SPOT_LIGHTS);
Texture2D<Vec4> g_shadowAtlasTex : register(ANKI_MATERIAL_REGISTER_SHADOW_ATLAS);
#endif

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

UnpackedMeshVertex loadVertex(MeshletGeometryDescriptor meshlet, U32 vertexIndex, Bool bones)
{
	UnpackedMeshVertex v;
	v.m_position = g_unifiedGeom_R16G16B16A16_Unorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kPosition] + vertexIndex];
	v.m_position = v.m_position * meshlet.m_positionScale + meshlet.m_positionTranslation;

	v.m_normal = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kNormal] + vertexIndex].xyz;
	v.m_uv = g_unifiedGeom_R32G32_Sfloat[meshlet.m_vertexOffsets[(U32)VertexStreamId::kUv] + vertexIndex];

	if(bones)
	{
		v.m_boneIndices = g_unifiedGeom_R8G8B8A8_Uint[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneIds] + vertexIndex];
		v.m_boneWeights = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneWeights] + vertexIndex];
	}

	return v;
}

Bool cullBackfaceMeshlet(MeshletBoundingVolume meshlet, Mat3x4 worldTransform, Vec3 cameraWorldPos)
{
	const Vec4 coneDirAndAng = unpackSnorm4x8(meshlet.m_coneDirection_R8G8B8_Snorm_cosHalfAngle_R8_Snorm);
	return cullBackfaceMeshlet(coneDirAndAng.xyz, coneDirAndAng.w, meshlet.m_coneApex, worldTransform, cameraWorldPos);
}
