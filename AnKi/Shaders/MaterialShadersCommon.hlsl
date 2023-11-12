// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common stuff for GBuffer, Forward shading and the rest of shaders that appear in materials.

#pragma once

#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/MeshTypes.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>

ANKI_BINDLESS_SET(MaterialSet::kBindless)

[[vk::binding(MaterialBinding::kTrilinearRepeatSampler, MaterialSet::kGlobal)]] SamplerState g_globalSampler;
[[vk::binding(MaterialBinding::kGlobalConstants, MaterialSet::kGlobal)]] ConstantBuffer<MaterialGlobalConstants> g_globalConstants;
[[vk::binding(MaterialBinding::kGpuScene, MaterialSet::kGlobal)]] ByteAddressBuffer g_gpuScene;

// Unified geom:
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType) \
	[[vk::binding(MaterialBinding::kUnifiedGeometry_##fmt, MaterialSet::kGlobal)]] Buffer<shaderType> g_unifiedGeom_##fmt;
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.defs.h>

[[vk::binding(MaterialBinding::kMeshlets, MaterialSet::kGlobal)]] StructuredBuffer<Meshlet> g_meshlets;
[[vk::binding(MaterialBinding::kTaskShaderPayloads, MaterialSet::kGlobal)]] StructuredBuffer<GpuSceneTaskShaderPayload> g_taskShaderPayloads;
[[vk::binding(MaterialBinding::kRenderables, MaterialSet::kGlobal)]] StructuredBuffer<GpuSceneRenderable> g_renderables;

// FW shading specific
#if defined(FORWARD_SHADING)
#	include <AnKi/Shaders/ClusteredShadingFunctions.hlsl>

[[vk::binding(MaterialBinding::kLinearClampSampler, MaterialSet::kGlobal)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(MaterialBinding::kDepthRt, MaterialSet::kGlobal)]] Texture2D g_gbufferDepthTex;
[[vk::binding(MaterialBinding::kLightVolume, MaterialSet::kGlobal)]] Texture3D<RVec4> g_lightVol;
[[vk::binding(MaterialBinding::kShadowSampler, MaterialSet::kGlobal)]] SamplerComparisonState g_shadowSampler;

[[vk::binding(MaterialBinding::kClusterShadingConstants, MaterialSet::kGlobal)]] ConstantBuffer<ClusteredShadingConstants> g_clusteredShading;
[[vk::binding(MaterialBinding::kClusters, MaterialSet::kGlobal)]] StructuredBuffer<Cluster> g_clusters;
[[vk::binding(MaterialBinding::kClusterShadingLights, MaterialSet::kGlobal)]] StructuredBuffer<PointLight> g_pointLights;
[[vk::binding(MaterialBinding::kClusterShadingLights, MaterialSet::kGlobal)]] StructuredBuffer<SpotLight> g_spotLights;
[[vk::binding((U32)MaterialBinding::kClusterShadingLights + 1u, MaterialSet::kGlobal)]] Texture2D<Vec4> g_shadowAtlasTex;
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

UnpackedMeshVertex loadVertex(Meshlet meshlet, U32 vertexIndex, Bool bones, F32 positionScale, Vec3 positionTranslation)
{
	UnpackedMeshVertex v;
	v.m_position = g_unifiedGeom_R16G16B16A16_Unorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kPosition] + vertexIndex];
	v.m_position = v.m_position * positionScale + positionTranslation;

	v.m_normal = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kNormal] + vertexIndex].xyz;
	v.m_uv = g_unifiedGeom_R32G32_Sfloat[meshlet.m_vertexOffsets[(U32)VertexStreamId::kUv] + vertexIndex];

	if(bones)
	{
		v.m_boneIndices = g_unifiedGeom_R8G8B8A8_Uint[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneIds] + vertexIndex];
		v.m_boneWeights = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneWeights] + vertexIndex];
	}

	return v;
}
